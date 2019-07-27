#include "includes.h"
/*-----------------------------------------------------------
 //(��λ������Ч && �ڱۿ�����Ч && ����������)==>��Ϊ��һ�������¼�
 //����>6��ʧ�ܺ�,�ȴ�2Сʱ�����¿�ʼ���������
 //���ֺ�,eject �澯��־��λ==>10���Ӻ���0
------------------------------------------------------------*/
static U8 _clec[CHANNEL_MAX];      //ȥ������
static U8 _cleec[CHANNEL_MAX];     //����ʧ�ܼ���
static int _cle10min[CHANNEL_MAX];     //10���Ӽ�ʱ,��
static int _cle2hour[CHANNEL_MAX];     //2Сʱ��ʱ,��

#define clec _clec[i-1]
#define cleec _cleec[i-1]
#define cle10min _cle10min[i-1]
#define cle2hour _cle2hour[i-1]

void channel_lock_timer(int ms)
{
	U8 i = 1;
	static int t = 0;
	t+=ms;
	if(t>=10000)//10��
	{
		t-=10000;
		for(;i<=CHANNEL_MAX;i++)
		{
			//10���ӣ�
			if(cle10min>0)
			{
				cle10min-=10;
				if(cle10min<=0)
				{
					Channel*pch = channel_data_get(i);
					cle10min=0;
					if(pch)pch->warn.eject=0;//�¼���־��0					
				}
			}
			if(cle2hour>0){
				cle2hour-=10;
				if(cle2hour<=0)
				{
					clec=cleec=cle2hour=0;//���¿�ʼ
				}
			}
		}
	}
}

/*------  ���� -- ���� -------*/
//������̣����ܳ��� �ڱۿ��� ����
AUTOSTART_THREAD_WITH_TIMEOUT(eject)
{
	static Channel*pch;
	static U8 i = 0;
	PROCESS_BEGIN();
  while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			pch=channel_data_get(i);
			if( (isvalid_daowe()==0) && (isvalid_baibi()==1) && (is_readok()==0) && (cleec <EJECT_FAIL_TIMES) )//(��λ������Ч && �ڱۿ�����Ч && ����������)
			{
				delayus(300);//ȥ��
				if( (isvalid_daowe()==0) && (isvalid_baibi()==1) && (is_readok()==0) )
				{
					clec++;
					if(clec>=2)
					{
						clec=0;
						request_charge_hangup_all(1); //�������г��
						os_delay(eject,10);//��ʱ10ms
						dian_ci_fa(pch,HIGH);//��ŷ�����
						os_delay(eject,500);//�ȴ�500ms
						dian_ci_fa(pch,LOW);//��ŷ�����
						pch->warn.eject = 1;//�¼�λ��1
						cle10min=600;       //�¼�λ��0,��ʱ10���Ӻ�eject��0
            os_delay(eject,50); //��ʱ
            if(isvalid_baibi()) //�ڱۿ��ػ���Ч��ʾû�е��ֳɹ�
						{
							pch->error.motor = 1;//��ŷ�����
							cleec++;          //�������++
							if(cleec==EJECT_FAIL_TIMES)cle2hour=EJECT_FAIL_WAIT;//�ȴ�2Сʱ
						}
						else 
						{
							cleec=0;	
							pch->error.motor = 0;//��ŷ�������0
						}							
					}
				}
			}
		}
		
		//10���Ӽ��һ�� EJECT_INTERVAL==10000
		{
			static time_t last = 0;last=time(0)+EJECT_INTERVAL;          //��һ��ʱ��
			do{os_delay(eject,1000);}while( (time(0)-last)> 0x80000000 );//��ʱ���
		}
	}
	PROCESS_END();
}
