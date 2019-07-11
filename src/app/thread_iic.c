#include "contiki.h"





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


AUTOSTART_PROCESSES(iic_thread);