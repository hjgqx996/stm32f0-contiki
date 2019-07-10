#include "contiki.h"
#include "thread_return.h"


/*===================================================
                本地函数
====================================================*/


/*===================================================
                全局函数
====================================================*/
static struct etimer et_return;
PROCESS(return_thread, "归还任务");
PROCESS_THREAD(return_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		 etimer_set(&et_return, CLOCK_SECOND);              // etimer溢出时间为5s
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_return));  // 等待定时器溢出
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(return_thread);