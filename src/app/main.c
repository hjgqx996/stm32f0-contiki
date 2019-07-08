

#include "contiki.h"
unsigned int idle_count = 0; 


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


