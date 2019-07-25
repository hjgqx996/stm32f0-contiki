#include "includes.h"


/*--------------------------------------------------------
1.对所有通道检查，看充电宝是否有效
2.有效的充电宝执行以下操作:
  (1) 读ID 
  (2) 读数据
  (3) 读输出标志
  (4) 6代宝以上,非租借条件下，应该 加密
								
								return: 0:正在运行 1:失败  2:成功
---------------------------------------------------------*/
static void read_data(Channel*pch,U8 ch)
{
	extern BOOL is_system_in_return(U8 addr);
	int err=0;
	#define t err
	U8 dataout[13];
  
	fsm_time_set(time(0));
	if(pch==NULL)return;
	if((is_system_in_return(pch->addr)==TRUE) )return;//当前是归还仓道，不读
	
	//根据失败次数，判断成功 or 失败
	if(pch->readerr>=BAO_READ_ERROR_RETYR_TIMES) 
	{
		channel_data_clear(ch);
		pch->readok=0; pch->readerr=0;pch->state.read_error=1;pch->state.read_ok=0;
	}
	if(pch->readok>=BAO_READ_OK_RETYR_TIMES)     
	{
		pch->readerr=0;pch->readok=0;pch->state.read_error=0;pch->state.read_ok=1;
	}
	
	/*摆臂开关有效可以读数据*/
	if(isvalid_baibi())
	{
		delayms(2);
		if(isvalid_baibi())
		{
			//读id
			if(channel_read(pch,RC_READ_ID,dataout,1000,FALSE)==FALSE)
			{
				//读不到数据
				pch->readerr++;
				return;
			}else{
				//读到数据
				pch->readok++;
			}
		  
			//读数据
			if(channel_read(pch,RC_READ_DATA,dataout,1000,FALSE)==FALSE)
			{
				//读不到数据
				pch->readerr++;
				return;
			}else{
				//读到数据
				pch->readok++;
			}		

      //加密
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
	/*摆臂开关无效数据清0*/
	else {
		if(!isvalid_baibi())
			channel_data_clear(ch);
	}
}


/*===================================================
						仓道任务: 读数据
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
			/*=====================状态位检测=========================*/
								channel_state_check(i);
			
			/*=====================告警位检测=========================*/
								channel_warn_check(i);
			
			/*=====================错误位检测=========================*/ 
								channel_error_check(i);		
			
			/*=====================系统灯=============================*/		
				pch = channel_data_get(i);
					if(pch==NULL)continue; 
			
				if(pch->error.baibi || pch->error.daowei )ld_system_flash_led(100); //开关错误，100ms     //心跳包500ms
				if( (time(0)/1000)%10==0 )ld_system_flash_led(2000);                 //10秒后复位为 2秒闪烁 	
			
			/*=====================读取充电宝=========================*/
					if(pch==NULL)continue; 
					read_data(pch,i);
			    os_delay(channel,50);
		}
		/*-----------循环等待时间---------------*/
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






