#include "includes.h"


void charge_fsm(U8 ch,void*arg);
/*===================================================
                �������
 �ϸ��� <ͨ������籦�����߼�.pdf> ���̿���								
====================================================*/
#define timeout(x)  (x<=0)
#define wait_timeout(condition,to,linenow)  end=to;line=linenow+1;case linenow+1: \
														                if((end>0) && (!(condition)) )return;
																						
/*�ϵ��⣬����������г�籦�������������*/
void recover_when_powerup(void)
{
	int i = 0;
	for(i=1;i<=CHANNEL_MAX;i++)
	{
		Channel*pch = channel_data_get(i);
		if(pch==NULL)continue;
		if( isvalid_baibi() )
		{
			charge_fsm(i,(void*)0x99);
		}
	}
}
/*-----------------------------
   ״̬������
-------------------------------*/
static U8   _line[CHANNEL_MAX]={0};//��ǰ״̬
static U8   _last[CHANNEL_MAX]={0};//��һ��״̬          :counter

static int  _end[CHANNEL_MAX]={0}; //��ʱ ms             :wait_timeout
static int  _timeout[CHANNEL_MAX]={0};//�� 120�������ʱ :ato
static BOOL _hangup[CHANNEL_MAX]={FALSE};//��ʱ����      :hang
static BOOL _request[CHANNEL_MAX]={FALSE};//������     :request
static int  _btimeout[CHANNEL_MAX]={0};//�����ʱ        :bto
static int  _failcounter[CHANNEL_MAX]={0};     //��籦�����ڼ���:failcounter
	
#define line    _line[ch-1]    //״̬����ǰ�к�
#define end     _end[ch-1]     //ms��ʱ
#define last    _last[ch-1]    //��һ��״̬
#define ato     _timeout[ch-1] //120���ʱ
#define hang    _hangup[ch-1]  //����:TRUE ==>ֹͣ��ʱ
#define counter _last[ch-1]    // 1Сʱ���һ�Σ���3�� �Ĵ�������
#define bto  _btimeout[ch-1]   // 3Сʱ��ʱ 1Сʱ��ʱ��,��
#define request _request[ch-1] //�Ƿ������˳��: 7Сʱ��� ����ʱ��
#define failcounter     _failcounter[ch-1]


static U8 current_read = 0;     //��ǰ�ֵ�
static U8 buffer[16];
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
		if(end>0)end-=ms; //����ʱ
		if(is_second)
		{
			if(ato)ato--;
			if(bto)bto--;
		}
	}
}

/*------------------------------------------------------------------------------------------------------
   �������
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
void charge_fsm(U8 ch,void*arg)
{
	extern BOOL is_system_in_return(U8 addr);
	Channel*pch=channel_data_get(ch);	      //�ֵ�����
	if(pch==NULL)return; 

	
	if((is_system_in_return(pch->addr)==TRUE) )return;//��ǰ�ǹ黹�ֵ�������(���й黹�߳��ڶ�)
	
	//�����û�е���
	if( isvalid_baibi()==0 && line !=0 )
	{
		failcounter++;	  //��������
		if(failcounter>3)
		{
			failcounter = line=0;request=FALSE;
			request_charge_off(ch);
			return;//�ֵ��Ѿ�����,��λ 
		}
	}  
	else { failcounter=0;}
	
	switch(line)
	{	
		//��ʼ(�ȴ��жϴ���/�ϵ紥��)
		case 0:
						last=end=ato=hang=0;
					  if((int)arg==0x99){channel_clear(ch); line=1;return;}   //�жϴ���,�ϵ紥��
			break;
						
		//����
		case 1:	if(isvalid_baibi()){line++; last=1;}return;//�ڱۿ��ؼ�⵽,��һ��
		
		//ʶ��/��ʶ��
		case 2:
						/*---------------��ʶ��---------------------------------*/
		        if( (channel_read(pch,RC_READ_ID,buffer,1000,TRUE)==TRUE) && (channel_read(pch,RC_READ_DATA,buffer,1000,TRUE)==TRUE) )
						{			
              pch->state.read_ok=1;							
							if(pch->Ufsoc>0){line=20;return;}  //����>0===>ֹͣ���
							else {
								line=10;
								request_charge_on(ch,600,TRUE);  //==>���10����
								return;
							}
							
						/*---------------����ʶ��---------------------------------*/
					 }else{	
						
						if(last==1){                        //��1������===>���5��
							line=4;
							request_charge_on(ch,5,TRUE);			//==>���5��			
						}
						
						else if(last==4){                   //�ӳ��5���������
							line=1;                           //���5���޷�ʶ�𣬸�λ
						}
						return;
					 }

		//���5��
		case 4:
						wait_timeout(isout5v(), 7*3600*1000, 4);//7Сʱ���Ƿ����(queue�Ŷ����)	
						if(isout5v())//�����
						{
							wait_timeout(0,5000, 6);//�ȴ�5��
							line=2;last=4;return;   //==����ʶ��һ��
						}
						else{line=1;return;}      //�����//��λ,��ͷ��ʼ 

			 
						
		//���10����
		case 10:
						wait_timeout(isout5v(), 7*3600*1000, 10);   //7Сʱ���Ƿ����(queue�Ŷ����)					
						if(isout5v())//�����
						{
							wait_timeout((pch->Ufsoc>0),600*1000, 12);//10���ӵ���>0
							line=20;                                  //��ʱ or ����>0 ===>ֹͣ���
							return;
						}
						//�����
						else{line=20;return;}     //�����==>ֹͣ���
						

		//ֹͣ���==>���7Сʱ
		case 20:
						request_charge_on(ch,7*3600,FALSE);          //���7Сʱ,endΪ��ʱʱ��
						line=24;ato=counter=bto=0;end =7*3600*1000;return;

						
		//���7Сʱ
	  case 24:
						if(timeout(end) || pch->state.full_charge)//����� or ��ʱ
						{
							request_charge_off(ch);//�����
							end=ato=counter=bto=0;line=26;//������(����׶�) 	
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
								end=ato=counter=bto=0;
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
						end=ato=counter=bto=0;
						request_charge_off(ch);
						line=30;request=FALSE;
						return;//������
						
					}
		break;
					
		//������
		case 30:
					request=FALSE;
		break;
		default:break;
	}
	
	  
		if(line>=24)
		{
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
	  }
		
		//�жϳ�����<100mA����2min
		if( (pch->AverageCurrent<STOP_CURRENT_MAX) && (line>=24) && (pch->state.full_charge!=1) && (pch->state.charging))
		{
			if(ato==0)ato=STOP_CURRENT_TIMEOUT;//��ʼ����ʱ
		}
		if( pch->AverageCurrent>=STOP_CURRENT_MAX)ato = 0;//�������ˣ���ʱ��λ
		
		if(ato==1 || ato==2)//���һ��ϳ�ʱ
		{
			pch->state.full_charge=1;//�Ѿ�����
			request_charge_off(ch);  //����ϵ�
			request=FALSE; 
			ato=0;
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
			charge_fsm(i,NULL);
		}
		os_delay(insert,50);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

