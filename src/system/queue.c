
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

/*互斥(contiki非抢占,所以不会多线程同时访问)*/
#define queue_lock()
#define queue_unlock()

#define l list[ch]

static BOOL inited = FALSE;

/*排队结构*/
#pragma pack(1)
typedef struct{
	U8 \
	used:1,    				//申请充电:1   未申请充电:0
	charge:1,  				//允许充电:1   不许充电:0
	hard:1,           //应急充电，不被打断
	inited:1,         //初始化
	reversed:4;				//未使用位
	U8 ch;            //仓道号 1-n
	U32 charge_time;	//充电时间(秒)  				
}Queue_Type;
#pragma pack()

static Queue_Type list[CHANNEL_MAX]={0,0,0,0,0,0,0,0};    //列表
/*===================================================
                本地函数
====================================================*/
/*冒泡排序:hard==1,value=200   hard==0,value=剩余电量*/
static void bubble_sort(void)
{
	int va = 0,vb=0;
	int i=0,j=0;
  Channel*pcha,*pchb;
	for(;i<CHANNEL_MAX-1;i++)
	{
		for(j=i+1;j<CHANNEL_MAX;j++)
		{
			pcha = channel_data_get(list[i].ch);
			pchb = channel_data_get(list[j].ch);
			if(pcha==NULL || pchb==NULL)continue;
			va= (list[i].used)?( (list[i].hard)?(200):(pcha->Ufsoc)):(0);  //应急充电在前
			vb= (list[j].used)?( (list[j].hard)?(200):(pchb->Ufsoc)):(0);  //之后，电量最大的依次排列
			if(va<vb)
			{
				//交换
				Queue_Type t = list[i];
				list[i] = list[j];
				list[j] = t;
			}
		}
	}
}

/*选择前面的充电,返回当前充电个数*/
static int charge_front(void)
{	
	int charge_counter=0; //当前正在充电的个数
	int ch=0;           
	Channel*pch;
	for(;ch<CHANNEL_MAX;ch++) //选择前面的充电
	{
		pch = channel_data_get(ch+1);
		if(pch==NULL)continue;
		if(l.used)
		{
			if(charge_counter<CHANNEL_CHARGE_MAX)
			{	
				charge_counter++;
				set_out5v();
			}else reset_out5v();
		}else reset_out5v();
	}	
}

/*计时*/
static void charge_timeout(void)
{
	static time_t t = 0;
	if(t==0)t=time(0)/1000;
	if( t != (time(0)/1000) )
	{
		int ch=0;           
		for(;ch<CHANNEL_MAX;ch++) //选择前面的充电
		{
			if(l.used)
			{
				if(l.charge_time!=0)
					l.charge_time--;
				else
					l.used=0;//停止充电
			}
		}		
	}
}

/*排队调度线程*/
AUTOSTART_THREAD_WITH_TIMEOUT(queue)
{
  
  memset(list,0,sizeof(list));	
	PROCESS_BEGIN();
	while(1)
	{
		bubble_sort();          //排序
		charge_timeout();			  //计时
		charge_front();         //前面先充
    os_delay(queue,20);     
	}

	PROCESS_END();
}

static void request_init(void)
{
	memset(list,0,sizeof(list));
	{
		int ch = 0;
		for(;ch<CHANNEL_MAX;ch++)
		{
			l.inited=1;
			l.ch=ch+1;
		}
	}
}
/*===================================================
                全局函数
====================================================*/

/*申请充电*/
BOOL request_charge_on(U8 ch,U32 seconds,BOOL hard)
{
	if(!inited)request_init();
	if(ch==0 || ch>= CHANNEL_MAX)return FALSE;ch--;
  
}

/*中止充电*/
BOOL request_charge_off(U8 ch)
{
	if(!inited){request_init();return TRUE;}

}

/*挂起充电*/
BOOL request_charge_hangup_all(U32 seconds)
{
	if(!inited){request_init();return TRUE;}
}
