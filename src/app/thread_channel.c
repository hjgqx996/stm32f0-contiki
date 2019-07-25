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
	int err=0;
	#define t err
	U8 dataout[13];
  
	fsm_time_set(time(0));
	if(pch==NULL)return;
	if((is_system_in_return(pch->addr)==TRUE) )return;//��ǰ�ǹ黹�ֵ�������
	
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
		delayms(2);
		if(isvalid_baibi())
		{
			//��id
			if(channel_read(pch,RC_READ_ID,dataout,1000,FALSE)==FALSE)
			{
				//����������
				pch->readerr++;
				return;
			}else{
				//��������
				pch->readok++;
			}
		  
			//������
			if(channel_read(pch,RC_READ_DATA,dataout,1000,FALSE)==FALSE)
			{
				//����������
				pch->readerr++;
				return;
			}else{
				//��������
				pch->readok++;
			}		

      //����
			if((is_ver_6() || is_ver_7()) && (is_system_lease()==FALSE) )
			{
				if(pch->bao_output!=0x06)
				{
					dataout[0]=0;
					channel_read(pch,RC_LOCK,dataout,800,FALSE);
					pch->bao_output=dataout[0];
				}
			}

		}
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
int channel_read_delay_ms = 0;
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	static U8 i = 0;
	static Channel*pch;
	PROCESS_BEGIN();          
	while(1)
	{
		channel_read_delay_ms = BAO_READ_DATA_MAX_MS;
		for(i=1;i<=CHANNEL_MAX;i++)
		{		
			/*=====================״̬λ���=========================*/
								channel_state_check(i);
			
			/*=====================�澯λ���=========================*/
								channel_warn_check(i);
			
			/*=====================����λ���=========================*/ 
								channel_error_check(i);		
			
			/*=====================ϵͳ��=============================*/		
				pch = channel_data_get(i);
					if(pch==NULL)continue; 
			
				if(pch->error.baibi || pch->error.daowei )ld_system_flash_led(100); //���ش���100ms     //������500ms
				if( (time(0)/1000)%10==0 )ld_system_flash_led(2000);                 //10���λΪ 2����˸ 	
			
			/*=====================��ȡ��籦=========================*/
					if(pch==NULL)continue; 
					read_data(pch,i);
			    os_delay(channel,50);
		}
		/*-----------ѭ���ȴ�ʱ��---------------*/
		if(channel_read_delay_ms>0)
		{
			os_delay(channel,channel_read_delay_ms);
		}
	  else 
		{
			os_delay(channel,100);
		}
		ld_iwdg_reload();	
	}
	PROCESS_END();
}






