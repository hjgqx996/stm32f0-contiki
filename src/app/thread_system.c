#include "includes.h"
/*===================================================
                系统任务
====================================================*/

AUTOSTART_THREAD_WITH_TIMEOUT(system)
{
	PROCESS_BEGIN();
	
	system.iic_ir_mode = SIIM_IIC_IR;
	ld_system_flash_led(2);
	while(1)
	{
		 os_delay(system,500);
	}

	PROCESS_END();
}





