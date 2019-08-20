#include "includes.h"
unsigned int idle_count = 0; 

/*自动运行线程地址*/
extern const  U32 autostart$$Base;
extern const  U32 autostart$$Limit;

int main(void)
{
	channel_data_init();//初始化仓道数据
	ld_system_init();   //系统参数  
	ld_dev_init();      //底层初始化
	clock_init();       //系统调度时钟
	process_init();     //线程初始化
	process_start(&etimer_process,NULL); //启动定时器
	autostart_start((struct process * const*)&autostart$$Base,((int)((int)&autostart$$Limit-(int)&autostart$$Base))/4);//自动运行线程
	while(1){
		while(process_run()> 0);//线程循环  
		idle_count++;  	
	}
//	return 0;
}


