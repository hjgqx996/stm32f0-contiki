#include "contiki.h"
#include "thread_return.h"


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
		 etimer_set(&et_return, CLOCK_SECOND);              // etimer���ʱ��Ϊ5s
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_return));  // �ȴ���ʱ�����
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(return_thread);