 #include "contiki.h"
#include "types.h"
#include "dev.h"

/*===================================================
                ϵͳ����
====================================================*/

static struct etimer et_system;
PROCESS(thread_system, "ϵͳ����");
PROCESS_THREAD(thread_system, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		 ld_gpio_set(1,0);
		 os_delay(et_system,500);
		 ld_gpio_set(1,1);
		 os_delay(et_system,500);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(thread_system);

