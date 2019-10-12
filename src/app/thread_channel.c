#include "includes.h"

extern void fsm_charge(U8 ch,int arg);
/*--------------------------------------------------------
1.对所有通道检查，看充电宝是否有效
2.有效的充电宝执行以下操作:
  (1) 读ID 
  (2) 读数据
  (3) 读输出标志
  (4) 6代宝以上,非租借条件下，应该 加密
								
	step:1 读id
	step:2 读数据
	step:3 加密
---------------------------------------------------------*/
static BOOL read_data(Channel*pch,U8 ch,U8 step)
{
	
	extern BOOL is_system_in_return(U8 addr);
	int result=0;
	U8 dataout[13];
  
	if(pch==NULL)return FALSE;
	if((is_system_in_return(pch->addr)==TRUE)  || (pch->first_insert!=FALSE) )return FALSE;//当前是归还仓道，不读   当前是第一次插入仓道不读
	
	//根据失败次数，判断成功 or 失败
	if(pch->readerr>=BAO_READ_ERROR_RETYR_TIMES) 
	{
		//调试信息
		#ifdef USING_DEBUG_INFO
		 if(channel_id_is_not_null(pch->id))ld_debug_printf(3,ch,0,pch->iic_ir_mode);
		#endif
		
		//清除仓道数据
		channel_data_clear(ch);
		pch->readok=0; pch->readerr=0;pch->state.read_error=1;pch->state.read_ok=0;
		
		//充电宝休眠，加电再读
		#if POWERUP_WHILE_READ_ERROR==1
		if( (pch->one_day_counter<POWERUP_ONE_DAY) && (pch->counter_while_powerup<POWERUP_TIMES) ){
			fsm_charge(ch,0x99);          //充电状态机复位,重新进入流程
			pch->counter_while_powerup++;	
      pch->one_day_counter++;			
		}else
		#endif	
			fsm_charge(ch,0x88);          //充电状态机复位,不充电		
		
	}

	/*摆臂开关有效可以读数据*/
	if(isvalid_baibi())
	{
		delayms(1);
		if(isvalid_baibi())
		{
			//读id
			if(step==1)
			{
				if(channel_id_is_not_null(pch->id)==FALSE)
					result = channel_read(pch,RC_READ_ID,dataout,550,FALSE);//实测410ms
				else 
					result =TRUE;
				if(result==FALSE)
				{
					//读不到数据
					pch->readerr++;
					return FALSE;
				}else{
					//读到数据
					pch->readok++;
				}
		 }
		  
			//读数据
		 if(step==2)
		 {
			result = channel_read(pch,RC_READ_DATA,dataout,650,FALSE);//实测512ms
			if(result==FALSE)
			{
				//读不到数据
				pch->readerr++;
				return FALSE;
			}else{
				//读到数据
				fsm_charge(ch,0x87);                                     //通知充电状态机，我读到数据了
				if(channel_id_is_not_null(pch->id) && (pch->readok>=1) ) //判断是否读到id
				{
					pch->state.read_ok=1;                                  //成功读到数据
					pch->state.read_error = pch->readerr=0;                //错误计数清0
					
					//充电宝休眠，加电再读，清计数
					#if POWERUP_WHILE_READ_ERROR==1
						pch->counter_while_powerup=0;
					#endif
				}
				pch->readok=0;
			}	
		 }			

      //加密
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
	/*摆臂开关无效数据清0*/
	else {
		if(!isvalid_baibi())
		{
			delayms(1);
			if(!isvalid_baibi())
			{
				//充电状态机复位
				fsm_charge(ch,0x88);
				
				//调试信息
				#ifdef USING_DEBUG_INFO
				 if(channel_id_is_not_null(pch->id))ld_debug_printf(4,ch,0,pch->iic_ir_mode);
				#endif
				
				//清除数据
				channel_data_clear(ch);
				return FALSE;
			}
		}
	}
	return FALSE;
}


/*===================================================
						仓道任务: 读数据
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

			/*=====================读取充电宝=========================*/
			result = read_data(pch,i,1);//读id    
			ld_iwdg_reload();	
			/*=====================测试红外功能=======================*/
			#if NOT_USING_IR ==0
			if(result){
				//测试红外
				if( (!is_ver_5()) && (!is_ver_6()) && (channel_id_is_not_null(pch->id)))//读到id,id不是5代宝，不是6代宝，读一次红外
				{
					if(!is_ir_mode()){		
						//隔一段时间读一次红外，如果失败，就认为红外故障了
						U16 dataout[8];
						
						if(pch->test_ir_counter%3==0)//3次循环读一次红外
						{
							pch->test_ir_counter=0;
							if(channel_read_from_ir(pch,RC_READ_DATA,(U8*)dataout,650)==FALSE)      //读失败
							{
								/* 运维发现,经常性出现 "红外1故障" 的仓道异常，为此添加过滤*/
								if( (ld_ir_read_state()== -1)  && ( valid_time() < FILTER_TIME_MAX) ) //充电宝进入3分钟后,开始滤波   
								{
									pch->ir_error_counter=BAO_IR_ERROR_TIMES;//红外识别故障
								  pch->error.ir=1;
								}
							}else{
								pch->ir_error_counter=0;//读取正确，清标志
								pch->error.ir=0;
								pch->test_ir_counter++;
								goto WAIT_NEXT_DELAY;
							}
						}
						pch->test_ir_counter++;
					}
				}
				//重新读回iic,(当前是红外，当前不是强制使用IR)
				if(is_ir_mode()&&(system.iic_ir_mode!=SIIM_ONLY_IR))
				{		
					if(pch->test_iic_counter < 4)//连续三次iic都有问题，一直使用IR
					{
						U16 dataout[8];
						if(channel_read_from_iic(pch,RC_READ_DATA,(U8*)dataout)==TRUE)
						{
							pch->iic_error_counter=0;
							pch->error.thimble=0;
							pch->iic_ir_mode=RTM_IIC;//切换回IIC
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
			/*=====================读取充电宝数据=========================*/
			if(result){
				read_data(pch,i,2);//读数据
				ld_iwdg_reload();	
			  os_delay(channel,50);
			}
			
			WAIT_NEXT_DELAY:
			/*=====================加密充电宝=========================*/
			if(result)
			{
				read_data(pch,i,3);//加密
				ld_iwdg_reload();	
			}
			/*=====================充电宝休眠唤醒====================*/
			
			/*-----------循环等待时间---------------------------------*/
			if((result) && (channel_read_delay_ms>40)){}
			else 
				channel_read_delay_ms=50;              //无宝短延时
			os_delay(channel,channel_read_delay_ms);			
			ld_iwdg_reload();		
		}
		ld_iwdg_reload();	
	}
	PROCESS_END();
}






