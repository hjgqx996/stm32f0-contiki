
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
	used:1,    //已经使用:1   还未使用:0
	charge:1,  //允许充电:1   不许充电:0
	reversed:6;//未使用位
	Channel*ch;
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
/*注册一个仓道：我要充电
* 返回: TRUE:注册成功
*     : FALSE:注册失败
*/
BOOL queue_regist(Channel*ch)
{

}

/*
* 查询仓道是否可以充电
* 返回:-1:出错   0:不充  1:可以充电
*/
int queue_isok(Channel*ch)
{

}

/* 
* 删除注册中的仓道:我不想充电了
* 返回：TRUE :成功删除 
*     ：FALSE:删除失败
*/
BOOL queue_delete(Channel*ch)
{

}
