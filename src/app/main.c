

#include "contiki.h"
#include "types.h"

unsigned int idle_count = 0; 

extern const  U32 autostart$$Base;
extern const  U32 autostart$$Limit;

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



static struct etimer et_t1;
PROCESS(thread_t1, "进入流程");
AUTOSTART_PROCESSES(thread_t1);
PROCESS_THREAD(thread_t1, ev, data)  
{
	static int cc = 0;
	PROCESS_BEGIN();
	cc++;
	os_delay(et_t1,100000);
	return 4;
	PROCESS_END();
}


static struct etimer et_t2;
PROCESS(thread_t2, "进入流程");
AUTOSTART_PROCESSES(thread_t2);
PROCESS_THREAD(thread_t2, ev, data)  
{
	static int scc=0;
	PROCESS_BEGIN();
	while(1)
	{
		process_post(&thread_t1,0x25,"hello");
		scc++;
		os_delay(et_t2,1000);
	}
	PROCESS_END();
}



