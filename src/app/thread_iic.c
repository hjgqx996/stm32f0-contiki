#include "contiki.h"





PROCESS(iic_thread, "iic����");
PROCESS_THREAD(iic_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    //iic��ʽ��ȡ��籦����
	}

	PROCESS_END();
}


AUTOSTART_PROCESSES(iic_thread);