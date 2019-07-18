 #include "contiki.h"
#include "types.h"
#include "dev.h"

/*===================================================
                系统任务
====================================================*/

AUTOSTART_THREAD_WITH_TIMEOUT(system)
{
	PROCESS_BEGIN();
	while(1)
	{
		 ld_gpio_set(1,0);
		 os_delay(system,500);
		 ld_gpio_set(1,1);
		 os_delay(system,500);
	}

	PROCESS_END();
}





