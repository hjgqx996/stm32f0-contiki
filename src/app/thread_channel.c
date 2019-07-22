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
	/*���ݸ�λ*/  fsm->i=0; \
	/*����3��*/		while(fsm->i<times) \
								{ \
	/*����++*/			fsm->i++;fsm->j=timeout/20; \
									do{ \
										  waitmsx(20); \
	/*������*/					err=channel_read(pch,cmd,dataout); \
	/*�������*/				if(err>=2)break; \
											fsm->j--; \
	/*���γ�ʱ*/			}while(fsm->j!=0); \
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
	
	//����ʧ�ܴ������жϳɹ� or ʧ��
	if(pch->readerr>=BAO_READ_ERROR_RETYR_TIMES) 
	{
		channel_data_clear(ch);
		pch->readok=0; pch->readerr=0;pch->state.read_error=1;pch->state.read_ok=0;
	}
	if(pch->readok>=BAO_READ_OK_RETYR_TIMES)     
	{
		pch->readerr=0;pch->readok=0;pch->state.read_error=0;pch->state.read_ok=1;
	}
	
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
						pch->readok++;pch->readerr=0;goto data;
					},
					{pch->readerr++;},              //ʧ��
					{fsm->line=0;return;});         //����
			
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
			if((is_ver_6() || is_ver_7()) && (is_system_lease()==FALSE) )
			{
				if(pch->bao_output!=0x06)
				{
					/*------��id--1��--��ʱ1s---�ɹ���������-------------------ʧ�ܴ���++-------- ��ʧ�ܸ�λ״̬��*/
					read(RC_LOCK ,1     ,1000  ,{pch->bao_output=(BaoOutput)dataout[0];},{pch->readerr++;},{fsm->line=0;return;});
				}
			}
			
			/*��ʱ*/
			waitmsx(1000);
			
		  //��λ״̬������ͷ��ʼ
			memset(fsm,0,sizeof(FSM));
			return;
		}
		Default()
	}

	/*�ڱۿ�����Ч������0*/
	else {
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
	PROCESS_BEGIN();          
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{		
		/*=====================״̬λ���=========================*/
							channel_state_check(i);
		/*=====================�澯λ���=========================*/
							channel_warn_check(i);
		/*=====================����λ���=========================*/ 
							channel_error_check(i);		
		/*=====================��ȡ��籦=========================*/
			Channel*pch = channel_data_get(i);
				if(pch==NULL)continue; 
			  read_data_fsm(pch,i);
		/*=====================ϵͳ��=============================*/	
			if(pch->error.baibi || pch->error.daowei )ld_system_flash_led(100); //���ش���100ms
			if( (time(0)/1000)%5==0 )ld_system_flash_led(2000);                 //5
		}
	  os_delay(channel,10);
		ld_iwdg_reload();
		
	}
	PROCESS_END();
}

