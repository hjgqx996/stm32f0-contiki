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
			/*��־һ�µ�ǰΪ������״̬*/request=TRUE;line=nextline;return

/*===================================================
						  ˽�б���
====================================================*/
static U8   _line[CHANNEL_MAX]={0};       //��ǰ״̬
static U8   _last[CHANNEL_MAX]={0};       //��һ��״̬          
static int  _to[CHANNEL_MAX]={0};         //timeout��ʱ           
static int  _s120[CHANNEL_MAX]={0};       //�� 120�������ʱ 
static BOOL _hang[CHANNEL_MAX]={FALSE};   //��ʱ����     
static BOOL _request[CHANNEL_MAX]={FALSE};//������     
static int  _btimeout[CHANNEL_MAX]={0};   //�����ʱ      
static U8   buffer[16];                   //����籦ʱ���ݻ���

#define line     _line[ch-1]    //״̬����ǰ�к�
#define to       _to[ch-1]      //ms��ʱ
#define last     _last[ch-1]    //��һ��״̬
#define s120     _s120[ch-1]    //120���ʱ
#define hang    _hang[ch-1]     //����:TRUE ==>ֹͣ��ʱ
#define counter _last[ch-1]     // 1Сʱ���һ�Σ���3�� �Ĵ�������
#define bto     _btimeout[ch-1] // 3Сʱ��ʱ 1Сʱ��ʱ��,��
#define request _request[ch-1]  //�Ƿ������˳��

/*===================================================
			�ϵ��⣬����������г�籦�������������
====================================================*/																						
void recover_when_powerup(void)
{
	int i = 0;
	for(i=1;i<=CHANNEL_MAX;i++)
	{
		Channel*pch = channel_data_get(i);
		if(pch==NULL)continue;
		if( isvalid_baibi() )
		{ 
			void fsm_charge(U8 ch,int arg);
			fsm_charge(i,0x99);
		}
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
			if(s120)s120--;//120���ʱ
			if(bto)bto--;  //�����ʱ
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
void fsm_charge(U8 ch,int arg)
{
	int result = 0;
	Channel*pch=channel_data_get(ch);if(pch==NULL)return;//�ֵ�����

	if((is_system_in_return(pch->addr)==TRUE) )return;   //��ǰ�ǹ黹�ֵ�������(���й黹�߳��ڶ�)
	if(arg==0x88){line=0;return;}                        //��λ,arg=0x88

	/*-----------------------���״̬��-------------------------------------------*/
	switch(line)
	{	
		//��ʼ(�ȴ��жϴ���/�ϵ紥��==>arg==0x99)
		case 0:
						last=to=s120=hang=0;
					  if(arg==0x99){ pch->first_insert=TRUE; channel_clear(ch); line=1;}return;   //�жϴ���,�ϵ紥��(0x99),������,����line=1
						
		//����
		case 1:	
						if(isvalid_baibi()){
								line++; last=1;  //ʶ��һ��//�ڱۿ��ؼ�⵽,��һ��
						}
						else 
							return;//�ȴ��ڱۿ���
		
		//ʶ��/��ʶ��
		case 2:
						/*---------------��ʶ��---------------------------------*/
						                  result = channel_read(pch,RC_READ_ID,buffer,500,TRUE);    if(result==-1)return; //�������ߣ�æ������,ʶ��һ��
		        if(result==FALSE) result = channel_read(pch,RC_READ_ID,buffer,500,TRUE);    if(result==-1)return; //�������ߣ�æ������,��ʶ��һ��
		        if(result==TRUE) result = channel_read(pch,RC_READ_DATA,buffer,600,TRUE);   if(result==-1)return; //�������ߣ�æ������
		        if(result==TRUE)
						{			
							pch->first_insert=FALSE;
              pch->state.read_ok=1;							
							if(pch->Ufsoc>0){line=20;return;}  //����>0===>ֹͣ���
							else 
							{                                  
								request_charge_and_wait_timeout(600,TRUE,10);//��Ϊ0 ===>������10����,����line=10
							}
						/*---------------����ʶ��---------------------------------*/
					 }else{	
							pch->first_insert=FALSE;						 
							if(last==1)
							{                        
									request_charge_and_wait_timeout(5,TRUE,4); //��1������===>���5��(����line=4)
							}				
							else if(last==4){ line=1;}       //���5���޷�ʶ�𣬸�λ                  
					 }
					 return;
					 
		//���5��
		case 4:
						if(timeout(to))
						{
							line=2;last=4;      //==����ʶ��һ��
						}return;
						
		//���10����
		case 10:
			      if(timeout(to) || (pch->Ufsoc>0))
						{
							request=FALSE;request_charge_off(ch); line=20;//��ʱ or ����>0 ===>ֹͣ���
						}return;
						
		//ֹͣ���==>���7Сʱ
		case 20:
			      s120=counter=bto=0;
		        request_charge_and_wait_timeout(7*3600,FALSE,24);//���7Сʱ,����line=24

		//���7Сʱ
	  case 24:
						if(timeout(to) || pch->state.full_charge)//����� or ��ʱ
						{
							request_charge_off(ch);       //�����
							to=s120=counter=bto=0;line=26;//������(����׶�) 	
						}
		break;
						
		//������(����׶�)
		case 26:
					if( (pch->Ufsoc<BUCHONG_STOP_UFSOC_MAX) && (pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX))//85%-99%,���3�Σ�ʱ��1Сʱ
					{
						if(timeout(bto))//����
						{
							bto=3600;counter++;//��ʱʱ��1Сʱ
							if(counter>BUCHONG_1HOUR_TIMES)//3�κ�,������
							{
								to=s120=counter=bto=0;
								request_charge_off(ch);//����ϵ�
								request=FALSE;
								line=30;//������
								return;
							}
							request_charge_on(ch,bto,FALSE);//������
							request=TRUE;
						}
					}
					
					else if(pch->Ufsoc<=BUCHONG_1HOUR_STOP_UFSOC_MAX)//<=85%,���޲���/3hour
					{
						counter=0;
						if(timeout(bto))
						{
							bto=3*3600;
							request_charge_on(ch,bto,FALSE);
							request=TRUE;
						}
					}
					
					else if(pch->Ufsoc>=BUCHONG_STOP_UFSOC_MAX)
					{
						to=s120=counter=bto=0;
						request_charge_off(ch);
						line=30;request=FALSE;
						return;//������
						
					}
		break;
					
		//������
		case 30:
			    request_charge_off(ch);
					request=FALSE;
		break;
		default:break;
	}
	 
		//��������������ʱ�������ʱ
		if( ((isout5v()==0) && (request==TRUE)) )
			hang=TRUE;
		else 
			hang=FALSE;
		
		//��籦������,��λ����ͷ��ʼ
		if( isvalid_baibi() && is_readerr() )
		{
			request_charge_off(ch);
			line=1;
			return;
		}
		
		//�жϳ�����<100mA����2min
		if( (pch->AverageCurrent<STOP_CURRENT_MAX) && (line>=24) && (pch->state.charging))
		{
			if(s120==0)
				s120=STOP_CURRENT_TIMEOUT;            //��ʼ����ʱ
		}
		if( pch->AverageCurrent>=STOP_CURRENT_MAX)//�������ˣ���ʱ��λ
			s120 = 0;
		
		if(s120==1 || s120==2)                   //���һ���жϳ�ʱ
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
	
	os_delay(insert,500);
	recover_when_powerup();//�ϵ��⵽�Ѿ��б������������̳��
	
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			fsm_charge(i,NULL);
		}
		os_delay(insert,40);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

