#include "contiki.h"
#include "types.h"


/*===================================================
                ���غ���
====================================================*/


/*===================================================
                ȫ�ֺ���
====================================================*/
static struct etimer et_return;
PROCESS(return_thread, "�黹����");
PROCESS_THREAD(return_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    os_delay(et_return,10);
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(return_thread);