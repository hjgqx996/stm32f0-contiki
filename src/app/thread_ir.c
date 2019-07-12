#include "contiki.h"


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


//AUTOSTART_PROCESSES(ir_thread);