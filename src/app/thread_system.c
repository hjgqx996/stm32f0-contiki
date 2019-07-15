#include "contiki.h"
#include "types.h"


/*===================================================
                系统任务
====================================================*/

static struct etimer et_system;
PROCESS(system_thread, "通道任务");
PROCESS_THREAD(system_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{

		 ld_gpio_set(1,0);
		 os_delay(et_system,100);
		 ld_gpio_set(1,1);
		 os_delay(et_system,100);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(system_thread);

