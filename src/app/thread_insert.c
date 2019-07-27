#include "includes.h"


void charge_fsm(U8 ch,void*arg);
/*===================================================
                �������
 �ϸ��� <ͨ������籦�����߼�.pdf> ���̿���								
====================================================*/
#define timeout(x)  (x<=0)
#define wait_timeout(condition,to,linenow)  end=to;line=linenow+1;case linenow+1: \
    												                if((end>0) && (!(condition)) )return;
																						
//��:����һ�γ����ʱ��Ϊseconds��,�����Ա�����
#define request_charge_and_wait_timeout(seconds,hard,nextline)  \
			/*������*/                  request_charge_on(ch,seconds,hard); \
			/*���ó�ʱʱ��*/              end = time(0)+1000*seconds; \
			/*��־һ�µ�ǰΪ������״̬*/request=TRUE;line=nextline;return
																						
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
static BOOL is_insert=FALSE;
BOOL is_inserted(void)//�Ƿ��г�籦����
{
	return is_insert;
}

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
#define request _request[ch-1] //�Ƿ������˳��
#define failcounter     _failcounter[ch-1]

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
	int result = 0;
	extern BOOL is_system_in_return(U8 addr);
	Channel*pch=channel_data_get(ch);	      //�ֵ�����
	if(pch==NULL)return; 

	
	if((is_system_in_return(pch->addr)==TRUE) )return;//��ǰ�ǹ黹�ֵ�������(���й黹�߳��ڶ�)
	
	//�����û�е���
	if( isvalid_baibi()==0 && line !=0 )
	{
		failcounter++;	  //��������
		if(failcounter>100)
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
					  if((int)arg==0x99){ is_insert=TRUE; channel_clear(ch); line=1;}return;   //�жϴ���,�ϵ紥��
						
		//����
		case 1:	
						if(isvalid_baibi()){
								line++; last=1;  //ʶ��һ��//�ڱۿ��ؼ�⵽,��һ��
							is_insert=FALSE; 
						}
						else 
							return;//�ȴ��ڱۿ���
		
		//ʶ��/��ʶ��
		case 2:
						/*---------------��ʶ��---------------------------------*/
						                result = channel_read(pch,RC_READ_ID,buffer,500,TRUE);    if(result==-1)return; //�������ߣ�æ������
		        if(result==TRUE)result = channel_read(pch,RC_READ_DATA,buffer,600,TRUE);  if(result==-1)return; //�������ߣ�æ������
		        if(result==TRUE)
						{			
              pch->state.read_ok=1;							
							if(pch->Ufsoc>0){line=20;return;}  //����>0===>ֹͣ���
							else {                             //��Ϊ0 ===>������10����,����line=10
								request_charge_and_wait_timeout(600,TRUE,10);
							}
						/*---------------����ʶ��---------------------------------*/
					 }else{			
						if(last==1){                        //��1������===>���5��(����line=4)
								request_charge_and_wait_timeout(5,TRUE,4);
						}				
						else if(last==4){ line=1;}          //���5���޷�ʶ�𣬸�λ                  
					 }
					 return;
					 
		//���5��
		case 4:
						if(timeout(end))
						{
							line=2;last=4;      //==����ʶ��һ��
						}return;
						
		//���10����
		case 10:
			      if(timeout(end) || (pch->Ufsoc>0))
						{
							request=FALSE;request_charge_off(ch); line=20;//��ʱ or ����>0 ===>ֹͣ���
						}return;
						
		//ֹͣ���==>���7Сʱ
		case 20:
			      ato=counter=bto=0;
		        request_charge_and_wait_timeout(7*3600,FALSE,24);//���7Сʱ,����line=24

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
		os_delay(insert,10);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

