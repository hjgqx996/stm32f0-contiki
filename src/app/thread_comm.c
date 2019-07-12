#include "contiki.h"
#include "types.h"


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
		
    //os_delay(et_comm,5);
		PROCESS_WAIT_EVENT();
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(comm_thread);