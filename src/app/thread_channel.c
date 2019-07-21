#include "includes.h"


///*===================================================
//               ��,���ͣ�����
//====================================================*/
static FSM rdfsm[CHANNEL_MAX];
///*===================================================
//                ˽�к���
//====================================================*/
/*--------------------------------------------------------------

*	��������*
*	[0] �汾�� [1] ���� [2] �¶� [3] ������ [4-5] ѭ������ [6-7] ���� [8-9] ��о��ѹ [10-11] ���� (��λ��ǰ)
*---------------------------------------------------------------
*/
void save_data(Channel*ch,U8*data)
{
	if(ch==NULL||data==NULL)return;
	ch->Ver					= data[0];
	ch->Ufsoc				= data[1];
	ch->Temperature			= data[2];
	ch->CycleCount			= (((U16)data[5])<<8)|(data[4]);
	ch->RemainingCapacity	= (((U16)data[7])<<8)|(data[6]);
	ch->Voltage				= (((U16)data[9])<<8)|(data[8]);
	ch->AverageCurrent		= (((U16)data[11])<<8)|(data[10]);
}



//Լ�� i:���Դ��� j:��ʱ���� 
#define read(cmd,times,timeout,sucess,fail,error) \
	/*���ݸ�λ*/fsm->i=0;fsm->j=timeout/10; \
	/*����3��*/		while(fsm->i++<times) \
								{ \
									do{ \
	/*������*/					err=channel_read(pch,cmd,dataout); \
	/*�������*/				if(err>=2)break; \
	/*�ȴ����*/				waitms(10); \
	/*���γ�ʱ*/			}while(fsm->j--!=0); \
	/*�ɹ�*/				if(err==2) sucess \
	/*ʧ��*/				else fail \
	/*���ζ�ʧ��*/			if(fsm->i>=3)error \
								}


/*--------------------------------------------------------
1.������ͨ����飬����籦�Ƿ���Ч
2.��Ч�ĳ�籦ִ�����²���:
  (1) ��ID 
  (2) ������
  (3) �������־
  (4) 6��������,����������£�Ӧ�� ����
---------------------------------------------------------*/
static void read_data_fsm(Channel*pch,U8 ch)
{
	FSM*fsm = &rdfsm[ch-1];
	int err=0;
	U8 dataout[13];

	fsm_time_set(time(0));
	
	if(pch->readerr>BAO_READ_ERROR_RETYR_TIMES)pch->readok=0;
	if(pch->readok>=2){pch->readok=2;pch->readok=1;}
	
	/*�ڱۿ�����Ч���Զ�����*/
	if(isvalid_baibi())
	{
		Start(){
			if(isvalid_baibi()){memset(fsm,0,sizeof(FSM));goto id;}
		}
		//��id
		State(id)
		{
			read(RC_READ_ID,2,1000,             //��id,2��----��ʱ1s
					{
						memcpy(pch->id,dataout,10);   //�ɹ�
						pch->readok++;goto data;
					},
					{pch->readerr++;},              //ʧ��
					{fsm->line=0;return;});               //����
		}

		//������
		State(data)
		{
		  /*------��id------2��----��ʱ1s---�ɹ���������------------------------------------------ʧ�ܴ���++-------- ��ʧ�ܸ�λ״̬��*/
			read(RC_READ_DATA ,2     ,1000  ,{save_data(pch,dataout);pch->readok++; goto lock678;},{pch->readerr++;},{fsm->line=0;return;});
		}

		//���� 6,7,8����(�����������)
		State(lock678)
		{
			if(is_ver_6() || is_ver_7())
			{
				if(pch->bao_output!=0x06)
				{
					/*------��id--1��--��ʱ1s---�ɹ���������-------------------ʧ�ܴ���++-------- ��ʧ�ܸ�λ״̬��*/
					read(RC_LOCK ,1     ,1000  ,{pch->bao_output=dataout[0];},{pch->readerr++;},{fsm->line=0;return;});
				}
			}
			
		  //��λ״̬������ͷ��ʼ
			memset(fsm,0,sizeof(FSM));
			return;
		}
		Default()
	}

	/*�ڱۿ�����Ч������0*/
	else {
		delayms(1);
		if(!isvalid_baibi())
			channel_data_clear(ch);
	}
}


/*===================================================
						�ֵ�����: ������
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	U8 i = 0;
	static time_t t = 0;
	
	PROCESS_BEGIN();          
	while(1)
	{
		t=time(0);
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			Channel*pch = channel_data_get(i);
				if(pch==NULL)continue;                        
			read_data_fsm(pch,i);
		}
		
		//��ʧ�ܣ�����ʱ    ���ɹ�����С��ʱ2.8��
		if(t!=0) { t= (BAO_READ_DATA_MAX_MS -(time(0)>t?time(0)-t:(0xFFFFFFFF-t+time(0))))%BAO_READ_DATA_MAX_MS;}
		if(t>0){os_delay(channel,t);}
		ld_iwdg_reload();
		
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
		ld_iwdg_reload();
	}
	PROCESS_END();
}