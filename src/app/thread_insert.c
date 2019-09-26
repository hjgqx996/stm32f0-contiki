#include "includes.h"


extern BOOL is_system_in_return(U8 addr);
/*===================================================
							�����
====================================================*/
//�Ƿ�ʱ
#define timeout(x)  (x<=0)   // is time out
																				
//��:����һ�γ����ʱ��Ϊseconds��,�����Ա�����
#define request_charge_and_wait_timeout(seconds,hard,nextline)  \
			/*������*/                  request_charge_on(ch,seconds,hard); \
			/*���ó�ʱʱ��*/              to = time(0)+1000*seconds; \
			/*��־һ�µ�ǰΪ������״̬*/request=TRUE;goto nextline

//�¶ȹ�������
//����>=50% �¶�>50��,������
//����<50%  �¶�>55��,������
#define is_temperature_high()   ( (pch->Ufsoc>=50&&pch->Temperature>50) ||(pch->Ufsoc <50&&pch->Temperature>55)	)
/*===================================================
						  ˽�б���
====================================================*/
static U16   _line[CHANNEL_MAX]={0};       //��ǰ״̬
static U8   _last[CHANNEL_MAX]={0};       //��һ��״̬          
static int  _to[CHANNEL_MAX]={0};         //timeout��ʱ           
static int  _s120[CHANNEL_MAX]={0};       //�� 120�������ʱ 
static BOOL _hang[CHANNEL_MAX]={FALSE};   //��ʱ����     
static BOOL _request[CHANNEL_MAX]={FALSE};//������    
static BOOL _highTemp[CHANNEL_MAX]={FALSE};//���±�־
static int  _1hour[CHANNEL_MAX]={0};      //1Сʱ�����ʱ
static U8   _1hourcount[CHANNEL_MAX]={0}; //1Сʱ����
static int  _3hour[CHANNEL_MAX]={0};      //3Сʱ�����ʱ
static U8   buffer[16];                   //����籦ʱ���ݻ���

#define line     _line[ch-1]      //״̬����ǰ�к�
#define to       _to[ch-1]        //ms��ʱ
#define last     _last[ch-1]      //��һ��״̬
#define s120     _s120[ch-1]      //120���ʱ
#define hang    _hang[ch-1]       //����:TRUE ==>ֹͣ��ʱ
#define counter _1hourcount[ch-1] // 1Сʱ���һ�Σ���3�� �Ĵ�������
#define hour1   _1hour[ch-1]      // 1Сʱ����ʱ
#define hour3   _3hour[ch-1]      // 3Сʱ����ʱ
#define request _request[ch-1]    //�Ƿ������˳��
#define HTemp   _highTemp[ch-1]   //���¹����־

/*===================================================
			�ϵ��⣬����������г�籦�������������
====================================================*/																						
void recover_when_powerup(void)
{
	int trytimes = 0;
	while(trytimes < CHECK_TIMES_POWER_UP)
	{
		int i = 0;
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			Channel*pch = channel_data_get(i);
			if(pch==NULL)continue;
			if( isvalid_baibi() )
			{ 
				void fsm_charge(U8 ch,int arg);
				if(pch->state.read_ok==0 || pch->state.read_from_ir==1 || pch->error.thimble==1 )// �ϵ��Լ죬���iicʧ�ܣ�����4��
				{		
					if(pch->state.read_from_ir==1)delayms(250);
					fsm_charge(i,0x99);
					fsm_charge(i,NULL);
					fsm_charge(i,NULL);
					fsm_charge(i,NULL);
					ld_iwdg_reload();
				}
			}
		}
		trytimes++;
	}
}
/*===================================================
						  ״̬����ʱ����
====================================================*/
//��ʱʹ���ⲿ��ʱ��,ms:�ж�ʱ��
void charge_fms_timer(int ms)
{
	int ch = 1;
	static int second = 0;
	BOOL is_second = FALSE;
	
	//���ʱ
	second+=ms;
	if(second>1000)
	{	
		second-=1000;
		is_second=TRUE;
	}	
	
	for(ch=1;ch<=CHANNEL_MAX;ch++)
	{
		if(hang)continue;
		if(to>0)to-=ms;  //����ʱ
		if(is_second)
		{
			if(s120 >0)s120--;  //120���ʱ
			if(hour1>0)hour1--;//1Сʱ��ʱ
			if(hour3>0)hour3--;//3Сʱ��ʱ
		}
	}
}

/*------------------------------------------------------------------------------------------------------
   ������� �ϸ��� <ͨ������籦�����߼�.pdf> ���̿���		
�ϵ�/��籦exti=======>����===(�ڱ���Ч)=> ��λ
                        +
                      (��Ч)                                                                        __
                        +======>�ȴ�3��===>ʶ��===(yes)==>��������0===(yes)=======================>|  |
                                            +                 |                                    |  |
                     (����)�黹ͨ��<--------+                 |                                    |  |
                                           (no)===>y���5��===|=>ʶ��==(yes)===>��������0==(yes)==>|ͣ|
                                                              +                      +             |ֹ|
                                                             (no)===>���10����<====(no)           |��|
                                                                          +                        |��|
                                                                   (����>0 or ��ʱ)===============>|__|
                                                                                                     |
        +---(no)--->���7Сʱ<-----------------------------------------------------------------------+ 
        +               +
        +               +
         -------(����<100,120��)----(yes)--> ����-->(85-99,1Сʱ����1�Σ�����3��)-->(<85% 3Сʱһ�����޲���)-->(>=99,�������)
-------------------------------------------------------------------------------------------------------*/
#define start()           switch(line){case 0:
#define state(name)       break;name:line=__LINE__;return;case __LINE__:
#define defaultx()        break;default:{line=0;}break;}
void fsm_charge(U8 ch,int arg)
{
	int result = 0;
	Channel*pch=channel_data_get(ch);if(pch==NULL)return;//�ֵ�����

	if((is_system_in_return(pch->addr)==TRUE) )return;   //��ǰ�ǹ黹�ֵ�������(���й黹�߳��ڶ�)
	if(arg==0x88){line=0;request_charge_off(ch);return;} //��λ,arg=0x88,�����
  if(arg==0x99){if(pch->state.read_ok)return; pch->first_insert=TRUE; goto entry;}   //�жϴ���,�ϵ紥��(0x99),������,����line=1
	if(arg==0x87){if(line==0) goto stop_charge;else return;} //��籦����,�޷�ʶ��, �� ���� ����ʶ����==>ֱ������(ֹͣ���)stop_charge;
	/*-----------------------���״̬��-------------------------------------------*/
	//��ʼ(�ȴ��жϴ���/�ϵ紥��==>arg==0x99)
	start()
	{
		to=s120=hour1=hour3=counter=0; hang=FALSE;
		return;
	}
	
	/*================================״̬:����(�г�籦����/�ϵ�)==================================================*/
	state(entry)
	{
		if(isvalid_baibi()){last =1;goto identify;}//��last=1���һ�£�����һ��ʼ������
		else 
			return;//�ȴ��ڱۿ���
	}
	
	/*================================״̬:ʶ��/5�����ʶ��=========================================================*/
	state(identify)
	{
				/*---------------��ʶ��---------------------------------*/
													                result = channel_read(pch,RC_READ_ID,buffer,550,TRUE);                    	
				if(result==FALSE) {  delayms(100);result = channel_read(pch,RC_READ_ID,buffer,550,TRUE);} 
				if(result==TRUE)  {  
														result = channel_read(pch,RC_READ_DATA,buffer,650,TRUE);
					 if(result==FALSE){delayms(100);result = channel_read(pch,RC_READ_DATA,buffer,650,TRUE);}
				}
				
				if(result==TRUE)
				{			
					pch->first_insert=FALSE;
					pch->state.read_ok=1;							
					if(pch->Ufsoc>0){goto stop_charge;}  //����>0===>ֹͣ���
					else 
					{                                  
						request_charge_and_wait_timeout(POWERUP_TIME_0_UfSOC,TRUE,charge_10_min);//��Ϊ0 ===>������10����
					}
				/*---------------����ʶ��---------------------------------*/
			 }else{	
					pch->first_insert=FALSE;						 
					if(last==1)
					{                        
							request_charge_and_wait_timeout(POWERUP_TIME_5_SECONDS,TRUE,charge_5_second);//===>���5��
					}				
					else{ line=0;}//���5���޷�ʶ��==>��λ                
			 }
			 return;		
	}
	
	/*================================״̬:�ȴ����5��==============================================================*/
	state(charge_5_second)
	{
		if(timeout(to)){last=4;goto identify;}//��last=4���һ�£�����5���������
		return;
	}
	
	/*================================״̬:�ȴ����10����============================================================*/
	state(charge_10_min)
	{
		if(timeout(to) || (pch->Ufsoc>0))//��ʱ or ����>0 ===>ֹͣ���
		{
			request=FALSE;
			request_charge_off(ch);
			goto stop_charge;
		}return;
	}
	
	/*================================״̬:����������==============================================================*/
	state(charge_complete)
	{	
		hour1=hour3=counter=s120=0;
		request_charge_off(ch);
		request=FALSE;
		if(pch->Ufsoc<=BUCHONG_STOP_UFSOC_MAX)goto recharge;//��������һ�ν���ʱ==>���²���
	}
	
	/*================================״̬:ֹͣ���==================================================================*/
	state(stop_charge)
	{
		s120=counter=hour1=hour3=0;
		request_charge_and_wait_timeout(7*3600,FALSE,charge_7_hours);//==>���7Сʱ
	}
	
	
	/*================================״̬:7Сʱ������=============================================================*/
	state(charge_7_hours)
	{
		if(timeout(to) || pch->state.full_charge)//����� or ��ʱ
		{
			request_charge_off(ch);       //�����
			to=s120=counter=hour1=hour3=0;//������(����׶�)
			goto recharge;
		}		
	}
	
	/*================================״̬:����(����Ƿ񲹳�)========================================================*/
	state(recharge)
	{
		request_charge_off(ch);       //�����,����ǿ�����,��Ӧ�ϵ�
		request=FALSE;                
		if( (pch->Ufsoc<=BUCHONG_STOP_UFSOC_MAX) && (pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX) )//85%-99%,���3�Σ�ʱ��1Сʱ
		{
			hour1=3600;
			goto recharge_3_times;
		}
		
		if(pch->Ufsoc<=BUCHONG_1HOUR_STOP_UFSOC_MAX)//<=85%,���޲���/3hour
		{
			hour3=3600*3;
			goto recharge_all_time;
		}
		
		if(pch->Ufsoc>BUCHONG_STOP_UFSOC_MAX)//==100%,������
			goto charge_complete;
	}
	
	/*================================״̬:ÿ��1Сʱ 3�� ����=========================================================*/
	//85%-99% ����3��,�������ɺ󣬼�����0
	state(recharge_3_times)
	{
		if(counter<=3)
	  {//ֻ��3��
			if(timeout(hour1))//��ʱ����
			{
				counter++;
				hour1=3600;
				s120=0;
				request=TRUE;
				request_charge_on(ch,3600,FALSE);//������
			}		
		}//3�β��䣬��û�г���99%==>������
		else{
			hour1=s120=0;
			request=FALSE;
			request_charge_off(ch);
			goto charge_complete;
		}//����ϵ�

		//��������85%-99%,���¼���Ƿ񲹳�
		if(!((pch->Ufsoc<=BUCHONG_STOP_UFSOC_MAX) && (pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX)))
		{
			hour1=s120=counter=0;//�ֿ�������3�β�����
			goto recharge;
		}
	}
	/*================================״̬:ÿ��3Сʱ���޲���=========================================================*/
	state(recharge_all_time)
	{
		if(timeout(hour3))
		{
			hour3=3600*3;
			request=TRUE;s120=0;
			request_charge_on(ch,3600,FALSE);//������
		}
		//����>85%,�˳����޲���
	  if(pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX)
		{
		  hour3=s120=0;
			goto recharge;
		}
	}
	defaultx()
	
	
	/*================================��������������ʱ�������ʱ=====================================================*/
	if( ((isout5v()==0) && (request==TRUE)) )
		hang=TRUE;
	else 
		hang=FALSE;
	
	//��籦������,��λ����ͷ��ʼ
	if( isvalid_baibi() && is_readerr() )
	{
		request_charge_off(ch);
		line=0;
		return;
	}
	/*================================�¶ȹ���,�����ʱ,�ȴ��¶Ƚ���,�ָ����===========================================*/
	if(is_temperature_high())
	{
		if(request==TRUE && HTemp==FALSE)
		{
				hang=TRUE;                //����
				HTemp=TRUE;               //���±�־
				request_charge_hangup(ch);//�����ϵ�
		}
	}else{
		if(request==TRUE && HTemp==TRUE)
			request_charge_recovery(ch);//�ָ��Ŷ�
		HTemp=FALSE;                  //����±�־
	}
	
  /*================================�жϳ�����<100mA����2min,�ϵ�===================================================*/		
	if( (pch->AverageCurrent<STOP_CURRENT_MAX) && (line>=200) && (pch->state.charging) && (request==TRUE))//���ֵ���<100mA
	{
		if(s120==0)
			s120=STOP_CURRENT_TIMEOUT+10;            //��ʼ����ʱ
	}
	if( pch->AverageCurrent>=STOP_CURRENT_MAX)//���ֵ���>=100mA,��ʱ��λ
		s120 = 0;
	
	if(s120 <=10 && s120 >0 )                   //���һ���жϳ�ʱ
	{
		pch->state.full_charge=1;              //�Ѿ�����
		request_charge_off(ch);                //����ϵ�
		request=FALSE; 
		s120=0;
	}
}

///*===================================================
//                ȫ�ֺ���
//====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(insert)
{
	static int i= 0;
	PROCESS_BEGIN();
	recover_when_powerup();            //�ϵ��⵽�Ѿ��б������������̳��
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			fsm_charge(i,NULL);           //���״̬������
			ld_iwdg_reload();             //ι��
		}
		channel_data_map_init();        //2019-9-19: �˿����ó�ʼ��:��ʱ������ʱ���ų����ص��ڴ��еĶ˿����ó�������,��������ͨ������������
		                                //��õİ취�ǣ���Ⲣ��ʼ�����ã���С�������
		os_delay(insert,200);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

