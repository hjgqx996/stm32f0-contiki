
/*===================================================
功能:充电排队算法
算法描述:

仓道分类: A.电量为0仓道    B.7小时内充电   C.充电完成可能补充

          1.优先级 A类 > B类 >C类
					2.多个A类出现时，先到先得，排队
					3.多个B类出现时，按电量最大的排队，
					4.
====================================================*/
#include "includes.h"


/*最大仓道数*/
#define MAX_CHANNEL           5

/*允许同时充电的最大仓道数*/
#define MAX_CHANNEL_CHARGE    2

/*互斥(contiki非抢占,所以不会多线程同时访问)*/
#define queue_lock()
#define queue_unlock()


/*排队结构*/
typedef struct{
	U8 \
	used:1,    				//已经使用:1   还未使用:0
	charge:1,  				//允许充电:1   不许充电:0
	reversed:6;				//未使用位
	U16 charge_time;	//充电时间(秒)
	U8 ch;     				//仓道号 1-n
}Queue_Type;
static Queue_Type list[MAX_CHANNEL];    //列表
/*===================================================
                本地函数
====================================================*/
/*排队调度线程*/
AUTOSTART_THREAD_WITH_TIMEOUT(queue)
{
	//当前正在充电的个数
	static U8 charge_counter;   
  memset(list,0,sizeof(list));	
	PROCESS_BEGIN();
	while(1)
	{
		
    os_delay(queue,10);
	}

	PROCESS_END();
}
/*===================================================
                全局函数
====================================================*/
/*申请充电*/
BOOL request_charge_on(U8 ch,U16 time)
{


}

/*中止充电*/
BOOL request_charge_off(U8 ch)
{


}

/*挂起充电*/
BOOL request_charge_hangup(U8 ch,U16 time)
{}
	
BOOL request_charge_hangup_all(U16 ms)
{

}




