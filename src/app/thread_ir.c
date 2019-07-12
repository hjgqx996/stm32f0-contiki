#include "contiki.h"
#include "types.h"

static struct etimer et_ir;
PROCESS(ir_thread, "红外任务");
PROCESS_THREAD(ir_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    //红外方式读取充电宝数据
		os_delay(et_ir,100);
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(ir_thread);