#include "contiki.h"
#include "thread_channel.h"


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
		 etimer_set(&et_channel, CLOCK_SECOND);              // etimer溢出时间为5s
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_channel));  // 等待定时器溢出
     //充电宝是否有效
		
		 //读一次充电宝数据
		
		 //转换充电宝数据
		
		 //通道状态,告警，错误检测
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(channel_thread);