#include "contiki.h"
#include "dev.h"


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
	os_delay(et_comm,1000);//延时1秒再通讯
	ld_uart_init();
	ld_uart_open(2,115200,8,0,1,100,100);
	while(1)
	{
		
    //os_delay(et_comm,5);
		PROCESS_WAIT_EVENT();
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(comm_thread);