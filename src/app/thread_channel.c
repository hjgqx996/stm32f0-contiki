#include "contiki.h"
#include "thread_channel.h"


/*===================================================
                本地函数
====================================================*/


/*===================================================
                全局函数
====================================================*/
PROCESS(ir_thread, "红外任务");
PROCESS_THREAD(ir_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    //红外方式读取充电宝数据
	}

	PROCESS_END();
}


PROCESS(iic_thread, "iic任务");
PROCESS_THREAD(iic_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    //iic方式读取充电宝数据
	}

	PROCESS_END();
}



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
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(channel_thread);