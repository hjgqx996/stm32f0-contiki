#include "contiki.h"
#include "types.h"


/*===================================================
                本地函数
====================================================*/


/*===================================================
                全局函数
====================================================*/

static struct etimer et_channel;
PROCESS(channel_thread, "通道任务");
PROCESS_THREAD(channel_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
     //充电宝是否有效
		
		 //读一次充电宝数据
		
		 //转换充电宝数据
		
		 //通道状态,告警，错误检测
		
		 //延时10ms
		 os_delay(et_channel,10);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(channel_thread);