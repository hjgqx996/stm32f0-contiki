#include "contiki.h"
#include "dev.h"


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
	os_delay(et_comm,1000);//��ʱ1����ͨѶ
	ld_uart_init();
	ld_uart_open(2,115200,8,0,1,100,100);
	while(1)
	{
		
    //os_delay(et_comm,5);
		PROCESS_WAIT_EVENT();
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(comm_thread);