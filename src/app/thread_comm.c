#include "contiki.h"



/*===================================================
                本地函数
====================================================*/


/*===================================================
                全局函数
====================================================*/
static struct etimer et_comm;
PROCESS(comm_thread, "通讯任务");
PROCESS_THREAD(comm_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		 etimer_set(&et_comm, CLOCK_SECOND);              // etimer溢出时间为5s
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_comm));  // 等待定时器溢出
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(comm_thread);