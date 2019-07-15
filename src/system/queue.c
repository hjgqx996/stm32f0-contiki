
/*===================================================
功能:充电排队算法
算法描述:

仓道分类: A.电量为0仓道    B.7小时内充电   C.充电完成可能补充

          1.优先级 A类 > B类 >C类
					2.多个A类出现时，先到先得，排队
					3.多个B类出现时，按电量最大的排队，
					4.
====================================================*/
#include "lib.h"
#include "channel.h"
#include "string.h"


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
//初始化
static void queue_init(void)
{
	memset(ready,0,sizeof(ready));
	memset(urgent,0,sizeof(urgent));
	memset(full,0,sizeof(full));
}
//插入一个列表
static BOOL queue_insert(Queue_Type*qt,Channel*ch)
{
	int i = 0;
	queue_lock();
	for(;i<MAX_CHANNEL;i++){
		if(qt[i].used==0){
			memset(&qt[i],0,sizeof(Queue_Type));
			qt[i].ch=ch;
			queue_unlock();
			return TRUE;
		}
	}
	queue_unlock();
	return FALSE;
}

//从一个列表删除
static BOOL quque_remove(Queue_Type*qt,Channel*ch)
{
  int i = 0;
	queue_lock();
	for(;i<MAX_CHANNEL;i++){
		if(qt[i].used==1&&qt[i].ch==ch){
			memset(&qt[i],0,sizeof(Queue_Type));
			queue_unlock();
			return TRUE;
		}
	}
	queue_unlock();
	return FALSE;
}

/*查找列表中的仓道*/
static Queue_Type*where_is_queue(Channel*ch)
{
	int i = 0;
	Queue_Type*qt=NULL;
	queue_lock();
	for(;i<MAX_CHANNEL;i++){
		if(ready[i].used==1&&ready[i].ch==ch){qt=&ready[i];goto WHERE_END;}
		if(urgent[i].used==1&&urgent[i].ch==ch){qt=&urgent[i];goto WHERE_END;}
		if(full[i].used==1&&full[i].ch==ch){qt=&full[i];goto WHERE_END;}
	}
			
	WHERE_END:
	queue_lock();
	return qt;
}
  

/*排队调度线程*/
#include "contiki.h"
static struct etimer et_queue;
PROCESS(thread_queue, "充电调度");
AUTOSTART_PROCESSES(thread_queue);
PROCESS_THREAD(thread_queue, ev, data)  
{
	//当前正在充电的个数
	static U8 charge_counter;                
	PROCESS_BEGIN();
	while(1)
	{
		
		//1.紧急列表,先入先充
		
		//2.就绪列表,冒泡排序,电量大的先充
		
		//3.完成列表,冒泡排序,电量小的先充
		
		//延时10ms
    os_delay(et_queue,10);
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
  //电量==0 放入:紧急列表
	//充电完成,放入:完成列表
	//其它:放入就绪列表 
	if(ch->Ufsoc==0)
		return queue_insert(urgent,ch);
	if(ch->cs.full_charge==1)
		return queue_insert(full,ch);
	else
		return queue_insert(ready,ch);
}

/*
* 查询仓道是否可以充电
* 返回:-1:出错   0:不充  1:可以充电
*/
int queue_isok(Channel*ch)
{
  Queue_Type*qt = where_is_queue(ch);
	if(qt==NULL)return -1;
	return qt->charge;
}

/* 
* 删除注册中的仓道:我不想充电了
* 返回：TRUE :成功删除 
*     ：FALSE:删除失败
*/
BOOL queue_delete(Channel*ch)
{
  Queue_Type *qt = where_is_queue(ch);
	queue_lock();
	if(qt==NULL){queue_unlock();return FALSE;}
	memset(qt,0,sizeof(Queue_Type));
	queue_unlock();
	return TRUE;
}
