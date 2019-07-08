


#include "contiki.h"

static struct etimer et_charge;
PROCESS(charge, "led_on");
PROCESS_THREAD(charge, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
	   etimer_set(&et_charge, CLOCK_SECOND);              // etimer���ʱ��Ϊ5s
       PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_charge));  // �ȴ���ʱ�����
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(&charge);