#include "contiki.h"
#include "types.h"


/*===================================================
                ���غ���
====================================================*/


/*===================================================
                ȫ�ֺ���
====================================================*/
static struct etimer et_return;
PROCESS(thread_return, "�黹����");
PROCESS_THREAD(thread_return, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    os_delay(et_return,10);
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(thread_return);