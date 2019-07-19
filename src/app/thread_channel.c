#include "includes.h"


///*===================================================
//               ��
//====================================================*/
#define output()         (buf[0])

#define channel_read_start_wait_end(cmd,sucess,to) \
          /*��ʼ������*/   error=0;timeoutcounter=to/10; \
					/*��ʼ������*/   do{error=channel_read(pch,cmd,buffer); \
					/*��ʱ10ms  */      os_delay(channel,10);timeoutcounter--; \
					/*��ȡ��ɻ�ʱ*/}while(error<2&&timeoutcounter>0);if(timeoutcounter<=0)error=4; \
															do{ \
					/*���гɹ�ʱִ��*/       if(error==2){sucess; \
																		pch->readok++; \
					/*����2�γɹ�,������*/    if(pch->readok>2)pch->readok=2; \
                                    pch->readerr=0; }else{ \
					/*ʧ�ܼ���*/              pch->readerr++; \
          /*ʧ�ܼ��Σ��Ͳ�����*/    if(pch->readerr>BAO_READ_ERROR_RETYR_TIMES) \
			    /*������0*/								{ pch->readok=0;channel_data_clear(i+1);	}} \
					                      }while(0);if(error!=2)t=0

///*===================================================
//                ˽�к���
//====================================================*/
/*��������*///[0] �汾�� [1] ���� [2] �¶� [3] ������ [4-5] ѭ������ [6-7] ���� [8-9] ��о��ѹ [10-11] ���� (��λ��ǰ)
static void save_data(Channel*ch,U8*data)
{
	if(ch==NULL||data==NULL)return;
	ch->Ver 							= data[0];
	ch->Ufsoc							=	data[1];
	ch->Temperature				=	data[2];
	ch->CycleCount				= (((U16)data[5])<<8)|(data[4]);
	ch->RemainingCapacity	= (((U16)data[7])<<8)|(data[6]);
	ch->Voltage          	= (((U16)data[9])<<8)|(data[8]);
	ch->AverageCurrent    = (((U16)data[11])<<8)|(data[10]);
}

/*===================================================
						�ֵ�����: ������
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	static U8 i = 0;
	static Channel *pch;
	static READ_TYPE_MODE mode;
	static U8 buffer[13];
  static U8 error;
	static int timeoutcounter;
	static time_t t = 0;
	PROCESS_BEGIN();
  channel_data_init();//��ʼ���ֵ�����
	while(1)
	{
		/*--------------------------------------------------------
		1.������ͨ����飬����籦�Ƿ���Ч
		2.��Ч�ĳ�籦ִ�����²���:
		  (1) ��ID 
		  (2) ������
		  (3) �������־
		  (4) 6��������,����������£�Ӧ�� ����
		---------------------------------------------------------*/
		t=time(0);
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			pch = channel_data_get(i);
			if(pch==NULL)continue;
			if(isvalid_baibi())															                         //�жϳ�籦�Ƿ���Ч
			{		
				channel_read_start_wait_end(RC_READ_ID,memcpy(pch->id,buffer,10),1000);//��ȡID	
				channel_read_start_wait_end(RC_READ_DATA, save_data(pch,buffer),1000); //��ȡ����

				if(is_ver_6() ||  is_ver_7())                                          //�����������(6�����ϣ���ֹ���,7������ʹ�ú���ͨѶ��ֹ���)
				{
					if((channel_id_is_not_null(pch->id)) && (pch->bao_output==0 || pch->bao_output==BAO_ALLOW || pch->bao_output==BAO_ALLOW_ONE_HOUR))
					{
						channel_read_start_wait_end(RC_LOCK, pch->bao_output=buffer[0],1000);	//���ܳ�籦
					}
				}
			}
			else//��籦������
			{
				if( (!isvalid_baibi())&&(!isvalid_daowe()) )
					channel_data_clear(i);//������0
			}
		}
		//��ʧ�ܣ�����ʱ    ���ɹ�����С��ʱ2.8��
		if(t!=0) { t= (BAO_READ_DATA_MAX_MS -(time(0)>t?time(0)-t:(0xFFFFFFFF-t+time(0))))%BAO_READ_DATA_MAX_MS;}
		if(t>0){os_delay(channel,t);}
		
	}
	PROCESS_END();
}

/*===================================================
						�ֵ�����: ʵʱ���ֵ�״̬
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel_state)
{
	static U8 i = 0;
	PROCESS_BEGIN();
	while(1)
	{
		for(i=0;i<CHANNEL_MAX;i++)
		{
		/*=====================״̬λ���=========================*/
							channel_state_check(i+1);
		/*=====================�澯λ���=========================*/
							channel_warn_check(i+1);
		/*=====================����λ���=========================*/ 
							channel_error_check(i+1);
		}
		os_delay(channel_state,100);
	}
	PROCESS_END();
}