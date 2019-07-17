#include "includes.h"

/*===================================================
                仓道任务
====================================================*/

static struct etimer et_channel;
PROCESS(channel_thread, "通道任务");
PROCESS_THREAD(channel_thread, ev, data)  
{
	static U32 timeout = 0;
	BOOL flag=FALSE;
	PROCESS_BEGIN();
  channel_data_init();//初始化仓道数据
	while(1)
	{
		U8 i = 0;
		
		//计时2.8秒
		if(timeout==0)timeout=time(0)+2800;
		if(!((time(0)-timeout)>0x80000000)){
			flag=TRUE;
			timeout=time(0)+2800;
		}
		
		for(;i<5;i++){
			//刷新仓道 状态，告警，异常
			channel_state_check(i);
			channel_warn_check(i);
			channel_error_check(i);
	
			//如果仓道有宝，读取仓道数据.频率2.8秒
			if(flag)
			{}
		}
		flag=FALSE;
		os_delay(et_channel,10);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(channel_thread);

/*===================================================
                充电宝读任务
====================================================*/
static struct etimer et_read;
PROCESS(thread_read, "充电宝读任务");
AUTOSTART_PROCESSES(thread_read);
PROCESS_THREAD(thread_read, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		PROCESS_WAIT_EVENT();
		switch(ev)
		{
			case PROCESS_EVENT_READ_ID:
				break;
			case PROCESS_EVENT_READ_DATA:
				break;
			case PROCESS_EVENT_READ_UNLOCK:
				break;
			case PROCESS_EVENT_READ_
		
		}
		//等待唤醒读取iic消息
		//ld_bq27541_read_id(21,22,ids);
		//ld_bq27541_check_ack(22,21);
	}

	PROCESS_END();
}
