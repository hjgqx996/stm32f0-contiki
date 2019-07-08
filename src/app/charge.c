


#include "contiki.h"

static struct etimer et_charge;
PROCESS(charge, "led_on");
PROCESS_THREAD(charge, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
	   etimer_set(&et_charge, CLOCK_SECOND);              // etimer溢出时间为5s
       PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_charge));  // 等待定时器溢出
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(&charge);