#include "contiki.h"
#include "types.h"
#include "dev.h"

/*===================================================
                ϵͳ����
====================================================*/

static struct etimer et_system;
PROCESS(system_thread, "ϵͳ����");
PROCESS_THREAD(system_thread, ev, data)  
{
	PROCESS_BEGIN();

	while(1)
	{
		 ld_gpio_set(1,0);
		 os_delay(et_system,500);
		 ld_gpio_set(1,1);
		 os_delay(et_system,500);
		 ld_gpio_set(2,1);//ʹ��485����
		 ld_uart_send(2,"kaihua",6);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(system_thread);

