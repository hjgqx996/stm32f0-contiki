#include "includes.h"
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
		#ifdef USING_DEBUG_INFO
		 if(channel_id_is_not_null(pch->id))ld_debug_printf(3,ch,0,pch->iic_ir_mode);
		#endif
		channel_data_clear(ch);
		pch->readok=0; pch->readerr=0;pch->state.read_error=1;pch->state.read_ok=0;
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
					return TRUE;
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
				return TRUE;
			}else{
				//读到数据
				{
					void fsm_charge(U8 ch,int arg);
					fsm_charge(ch,0x87);                                   //通知充电状态机，我读到数据了
				}
				if(channel_id_is_not_null(pch->id) && (pch->readok>=1) ) //判断是否读到id
				{
					pch->state.read_ok=1;                                  //成功读到数据
					pch->state.read_error = pch->readerr=0;                //错误计数清0
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
				extern void fsm_charge(U8 ch,int arg);
				fsm_charge(ch,0x88);//充电状态机复位
				
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
						仓道任务: 读数据
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

			/*=====================读取充电宝=========================*/
			result = read_data(pch,i,1);//读id    
			if(result){
				read_data(pch,i,2);//读数据
			  os_delay(channel,50);
			  read_data(pch,i,3);//加密
				os_delay(channel,50);
			}
			/*-----------循环等待时间---------------------------------*/
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






