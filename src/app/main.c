

#include "contiki.h"
unsigned int idle_count = 0; 

static struct etimer et_on;
PROCESS(led_on, "led_on");
PROCESS_THREAD(led_on, ev, data)  
{
    static int on = 0;
	PROCESS_BEGIN();

	while(1)
	{
	   on++;
	   etimer_set(&et_on, CLOCK_SECOND);              // etimer溢出时间为5s
       PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_on));  // 等待定时器溢出
	}

	PROCESS_END();
}


static struct etimer et_off;
PROCESS(led_off, "led_off"); 
PROCESS_THREAD(led_off, ev, data)  
{
    static int off = 0;
	PROCESS_BEGIN();

	while(1)
	{
	   off++;
	   etimer_set(&et_off, CLOCK_SECOND);              // etimer溢出时间为5s
       PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_off));  // 等待定时器溢出
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(&led_on,&led_off);  

int main(void*arg)
{
	clock_init();  
	process_init();  
	process_start(&etimer_process,NULL);  
	autostart_start(autostart_processes);
	while(1){
		while(process_run()> 0);  
		idle_count++;  
	}
}


