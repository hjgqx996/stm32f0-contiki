#include "contiki.h"
#include "types.h"


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
			{
				channel_read_data(i);
			}
		}
		flag=FALSE;
		os_delay(et_channel,10);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(channel_thread);