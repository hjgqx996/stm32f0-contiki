

#include "contiki.h"
#include "types.h"

unsigned int idle_count = 0; 

extern const  U32 autostart$$Base;
extern const  U32 autostart$$Limit;
extern const  U32 autostart$$Size;

int main(void)
{
	ld_dev_init();
	clock_init();  
	process_init();  
	process_start(&etimer_process,NULL);  
	autostart_start((struct process * const*)&autostart$$Base,((int)((int)&autostart$$Limit-(int)&autostart$$Base))/4);
	while(1){
		while(process_run()> 0);  
		idle_count++;  
	}
	
//	return 0;
}


