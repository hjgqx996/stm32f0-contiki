#include "includes.h"
/*--------------------------------------------------------
1.������ͨ����飬����籦�Ƿ���Ч
2.��Ч�ĳ�籦ִ�����²���:
  (1) ��ID 
  (2) ������
  (3) �������־
  (4) 6��������,����������£�Ӧ�� ����
								
								return: 0:�������� 1:ʧ��  2:�ɹ�
---------------------------------------------------------*/
static void read_data(Channel*pch,U8 ch)
{
	
	extern BOOL is_system_in_return(U8 addr);
	int result=0;
	U8 dataout[13];
  
	fsm_time_set(time(0));
	if(pch==NULL)return;
	if((is_system_in_return(pch->addr)==TRUE)  || (pch->first_insert==TRUE) )return;//��ǰ�ǹ黹�ֵ�������   ��ǰ�ǵ�һ�β���ֵ�����
	
	//����ʧ�ܴ������жϳɹ� or ʧ��
	if(pch->readerr>=BAO_READ_ERROR_RETYR_TIMES) 
	{
		channel_data_clear(ch);
		pch->readok=0; pch->readerr=0;pch->state.read_error=1;pch->state.read_ok=0;
	}

	/*�ڱۿ�����Ч���Զ�����*/
	if(isvalid_baibi())
	{
		delayms(2);
		if(isvalid_baibi())
		{
			//��id
			result = channel_read(pch,RC_READ_ID,dataout,500,FALSE);if(result==-1)return;//����æ������
			if(result==FALSE)
			{
				//����������
				pch->readerr++;
				return;
			}else{
				//��������
				pch->readok++;
			}
		  
			//������
			result = channel_read(pch,RC_READ_DATA,dataout,600,FALSE);if(result==-1)return;//����æ������
			if(result==FALSE)
			{
				//����������
				pch->readerr++;
				return;
			}else{
				//��������
				
				if(channel_id_is_not_null(pch->id) && (pch->readok>=1) ) //�ж��Ƿ����id
				{
					pch->state.read_ok=1;                                  //�ɹ���������
					pch->state.read_error = pch->readerr=0;                //���������0
				}
				pch->readok=0;
			}		

      //����
			if((is_ver_6() || is_ver_7()) && (is_system_lease()==FALSE) )
			{
				if(pch->bao_output!=0x06)
				{
					dataout[0]=0;
					channel_read(pch,RC_LOCK,dataout,600,FALSE);
					pch->bao_output=dataout[0];
				}
			}

		}
	}
	/*�ڱۿ�����Ч������0*/
	else {
		if(!isvalid_baibi())
			if(!isvalid_baibi())
			{
				charge_fsm(ch,(void*)0x88);//���״̬����λ
				channel_data_clear(ch);
			}
	}
}


/*===================================================
						�ֵ�����: ������
====================================================*/
int channel_read_delay_ms = BAO_READ_DATA_MAX_MS;
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	static U8 i = 0;
	static Channel*pch;
	PROCESS_BEGIN();          
	while(1)
	{
		
		for(i=1;i<=CHANNEL_MAX;i++)
		{	
			channel_read_delay_ms = BAO_READ_DATA_MAX_MS;	
			pch = channel_data_get(i);

			/*=====================��ȡ��籦=========================*/
				read_data(pch,i);
			/*-----------ѭ���ȴ�ʱ��---------------------------------*/
			if(channel_read_delay_ms>0)
			{
				os_delay(channel,channel_read_delay_ms);
			}
			else 
			{
				os_delay(channel,100);
			}				
				
		}

		ld_iwdg_reload();	
	}
	PROCESS_END();
}






