#include "contiki.h"
#include "types.h"

static struct etimer et_ir;
PROCESS(ir_thread, "��������");
PROCESS_THREAD(ir_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    //���ⷽʽ��ȡ��籦����
		os_delay(et_ir,100);
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(ir_thread);