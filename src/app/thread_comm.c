#include "contiki.h"



/*===================================================
                ���غ���
====================================================*/


/*===================================================
                ȫ�ֺ���
====================================================*/
static struct etimer et_comm;
PROCESS(comm_thread, "ͨѶ����");
PROCESS_THREAD(comm_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		 etimer_set(&et_comm, CLOCK_SECOND);              // etimer���ʱ��Ϊ5s
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_comm));  // �ȴ���ʱ�����
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(comm_thread);