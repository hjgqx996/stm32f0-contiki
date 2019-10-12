#include "includes.h"

extern void fsm_charge(U8 ch,int arg);
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
		//������Ϣ
		#ifdef USING_DEBUG_INFO
		 if(channel_id_is_not_null(pch->id))ld_debug_printf(3,ch,0,pch->iic_ir_mode);
		#endif
		
		//����ֵ�����
		channel_data_clear(ch);
		pch->readok=0; pch->readerr=0;pch->state.read_error=1;pch->state.read_ok=0;
		
		//��籦���ߣ��ӵ��ٶ�
		#if POWERUP_WHILE_READ_ERROR==1
		if( (pch->one_day_counter<POWERUP_ONE_DAY) && (pch->counter_while_powerup<POWERUP_TIMES) ){
			fsm_charge(ch,0x99);          //���״̬����λ,���½�������
			pch->counter_while_powerup++;	
      pch->one_day_counter++;			
		}else
		#endif	
			fsm_charge(ch,0x88);          //���״̬����λ,�����		
		
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
					return FALSE;
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
				return FALSE;
			}else{
				//��������
				fsm_charge(ch,0x87);                                     //֪ͨ���״̬�����Ҷ���������
				if(channel_id_is_not_null(pch->id) && (pch->readok>=1) ) //�ж��Ƿ����id
				{
					pch->state.read_ok=1;                                  //�ɹ���������
					pch->state.read_error = pch->readerr=0;                //���������0
					
					//��籦���ߣ��ӵ��ٶ��������
					#if POWERUP_WHILE_READ_ERROR==1
						pch->counter_while_powerup=0;
					#endif
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
				//���״̬����λ
				fsm_charge(ch,0x88);
				
				//������Ϣ
				#ifdef USING_DEBUG_INFO
				 if(channel_id_is_not_null(pch->id))ld_debug_printf(4,ch,0,pch->iic_ir_mode);
				#endif
				
				//�������
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
  os_delay(channel,500);	
	while(1)
	{
		
		for(i=1;i<=CHANNEL_MAX;i++)
		{	
			channel_read_delay_ms = BAO_READ_DATA_MAX_MS;	
			pch = channel_data_get(i);

			/*=====================��ȡ��籦=========================*/
			result = read_data(pch,i,1);//��id    
			ld_iwdg_reload();	
			/*=====================���Ժ��⹦��=======================*/
			#if NOT_USING_IR ==0
			if(result){
				//���Ժ���
				if( (!is_ver_5()) && (!is_ver_6()) && (channel_id_is_not_null(pch->id)))//����id,id����5����������6��������һ�κ���
				{
					if(!is_ir_mode()){		
						//��һ��ʱ���һ�κ��⣬���ʧ�ܣ�����Ϊ���������
						U16 dataout[8];
						
						if(pch->test_ir_counter%3==0)//3��ѭ����һ�κ���
						{
							pch->test_ir_counter=0;
							if(channel_read_from_ir(pch,RC_READ_DATA,(U8*)dataout,650)==FALSE)      //��ʧ��
							{
								/* ��ά����,�����Գ��� "����1����" �Ĳֵ��쳣��Ϊ����ӹ���*/
								if( (ld_ir_read_state()== -1)  && ( valid_time() < FILTER_TIME_MAX) ) //��籦����3���Ӻ�,��ʼ�˲�   
								{
									pch->ir_error_counter=BAO_IR_ERROR_TIMES;//����ʶ�����
								  pch->error.ir=1;
								}
							}else{
								pch->ir_error_counter=0;//��ȡ��ȷ�����־
								pch->error.ir=0;
								pch->test_ir_counter++;
								goto WAIT_NEXT_DELAY;
							}
						}
						pch->test_ir_counter++;
					}
				}
				//���¶���iic,(��ǰ�Ǻ��⣬��ǰ����ǿ��ʹ��IR)
				if(is_ir_mode()&&(system.iic_ir_mode!=SIIM_ONLY_IR))
				{		
					if(pch->test_iic_counter < 4)//��������iic�������⣬һֱʹ��IR
					{
						U16 dataout[8];
						if(channel_read_from_iic(pch,RC_READ_DATA,(U8*)dataout)==TRUE)
						{
							pch->iic_error_counter=0;
							pch->error.thimble=0;
							pch->iic_ir_mode=RTM_IIC;//�л���IIC
							pch->test_iic_counter =0;
							goto WAIT_NEXT_DELAY;
						}
					}
					if(pch->test_iic_counter<100)
						pch->test_iic_counter ++;
					 os_delay(channel,20);
				}
			}
			#endif
			/*=====================��ȡ��籦����=========================*/
			if(result){
				read_data(pch,i,2);//������
				ld_iwdg_reload();	
			  os_delay(channel,50);
			}
			
			WAIT_NEXT_DELAY:
			/*=====================���ܳ�籦=========================*/
			if(result)
			{
				read_data(pch,i,3);//����
				ld_iwdg_reload();	
			}
			/*=====================��籦���߻���====================*/
			
			/*-----------ѭ���ȴ�ʱ��---------------------------------*/
			if((result) && (channel_read_delay_ms>40)){}
			else 
				channel_read_delay_ms=50;              //�ޱ�����ʱ
			os_delay(channel,channel_read_delay_ms);			
			ld_iwdg_reload();		
		}
		ld_iwdg_reload();	
	}
	PROCESS_END();
}






