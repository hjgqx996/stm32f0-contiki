#include "includes.h"
/*--------------------------------------------------------
1.������ͨ����飬����籦�Ƿ���Ч
2.��Ч�ĳ�籦ִ�����²���:
  (1) ��ID 
  (2) ������
  (3) �������־
  (4) 6��������,����������£�Ӧ�� ����
								
	step:1 ��id
	step:2 ������
	step:3 ����
---------------------------------------------------------*/
static BOOL read_data(Channel*pch,U8 ch,U8 step)
{
	
	extern BOOL is_system_in_return(U8 addr);
	int result=0;
	U8 dataout[13];
  
	if(pch==NULL)return FALSE;
	if((is_system_in_return(pch->addr)==TRUE)  || (pch->first_insert!=FALSE) )return FALSE;//��ǰ�ǹ黹�ֵ�������   ��ǰ�ǵ�һ�β���ֵ�����
	
	//����ʧ�ܴ������жϳɹ� or ʧ��
	if(pch->readerr>=BAO_READ_ERROR_RETYR_TIMES) 
	{
		#ifdef USING_DEBUG_INFO
		 if(channel_id_is_not_null(pch->id))ld_debug_printf(3,ch,0,pch->iic_ir_mode);
		#endif
		channel_data_clear(ch);
		pch->readok=0; pch->readerr=0;pch->state.read_error=1;pch->state.read_ok=0;
	}

	/*�ڱۿ�����Ч���Զ�����*/
	if(isvalid_baibi())
	{
		delayms(1);
		if(isvalid_baibi())
		{
			//��id
			if(step==1)
			{
				if(channel_id_is_not_null(pch->id)==FALSE)
					result = channel_read(pch,RC_READ_ID,dataout,550,FALSE);//ʵ��410ms
				else 
					result =TRUE;
				if(result==FALSE)
				{
					//����������
					pch->readerr++;
					return TRUE;
				}else{
					//��������
					pch->readok++;
				}
		 }
		  
			//������
		 if(step==2)
		 {
			result = channel_read(pch,RC_READ_DATA,dataout,650,FALSE);//ʵ��512ms
			if(result==FALSE)
			{
				//����������
				pch->readerr++;
				return TRUE;
			}else{
				//��������
				{
					void fsm_charge(U8 ch,int arg);
					fsm_charge(ch,0x87);                                   //֪ͨ���״̬�����Ҷ���������
				}
				if(channel_id_is_not_null(pch->id) && (pch->readok>=1) ) //�ж��Ƿ����id
				{
					pch->state.read_ok=1;                                  //�ɹ���������
					pch->state.read_error = pch->readerr=0;                //���������0
				}
				pch->readok=0;
			}	
		 }			

      //����
		 if(step==3)
		 {
			if((!is_ver_5()) && (is_system_lease()==FALSE) )
			{
				if(pch->bao_output!=BAO_NOTALLOW)
				{
					dataout[0]=0;
					channel_read(pch,RC_LOCK,dataout,650,FALSE);
					pch->bao_output= (BaoOutput)dataout[0];
				}
			}
		 }
		}
		
		return TRUE;
	}
	/*�ڱۿ�����Ч������0*/
	else {
		if(!isvalid_baibi())
		{
			delayms(1);
			if(!isvalid_baibi())
			{
				extern void fsm_charge(U8 ch,int arg);
				fsm_charge(ch,0x88);//���״̬����λ
				
				#ifdef USING_DEBUG_INFO
				 if(channel_id_is_not_null(pch->id))ld_debug_printf(4,ch,0,pch->iic_ir_mode);
				#endif
				channel_data_clear(ch);
				return FALSE;
			}
		}
	}
	return FALSE;
}


/*===================================================
						�ֵ�����: ������
====================================================*/
int channel_read_delay_ms = BAO_READ_DATA_MAX_MS;
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	static U8 i = 0;
	static Channel*pch;
	static BOOL result = FALSE;
	PROCESS_BEGIN();          
	while(1)
	{
		
		for(i=1;i<=CHANNEL_MAX;i++)
		{	
			channel_read_delay_ms = BAO_READ_DATA_MAX_MS;	
			pch = channel_data_get(i);

			/*=====================��ȡ��籦=========================*/
			result = read_data(pch,i,1);//��id    
			if(result){
				read_data(pch,i,2);//������
			  os_delay(channel,50);
			  read_data(pch,i,3);//����
				os_delay(channel,50);
			}
			/*-----------ѭ���ȴ�ʱ��---------------------------------*/
			if((result) && (channel_read_delay_ms>0) )
			{
				os_delay(channel,channel_read_delay_ms);
			}
			else 
			{
				os_delay(channel,50);
			}				
			ld_iwdg_reload();		
		}
		ld_iwdg_reload();	
	}
	PROCESS_END();
}






