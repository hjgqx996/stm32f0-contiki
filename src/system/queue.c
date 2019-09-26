
#include "includes.h"

/*是否强制充电*/
#define is_force_charge_on(ch)    ((system.mode==1) && (system.chs[ch-1]==0x01))   //强制充电
#define is_force_charge_off(ch)   ((system.mode==1) && (system.chs[ch-1]==0x00))   //强制断电

/*互斥(contiki非抢占,所以不会多线程同时访问)*/
#define queue_lock()
#define queue_unlock()

#define l list[ch]
static BOOL inited = FALSE;   //是否初始化
static BOOL hangall = FALSE;  //是否挂起
static U32  hangtime = 0;     //挂起时间 ms

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
/*
*  只有一个hard在最前面:保证有一个宝在充电
*  [hard] [......] [hard][hard]...
*/
/*冒泡排序*/
static void bubble_sort(void)
{
	int va = 0,vb=0;
	int i=0,j=0;
  Channel*pcha,*pchb;
	BOOL first = FALSE;
	for(;i<CHANNEL_MAX-1;i++)
	{
		for(j=i+1;j<CHANNEL_MAX;j++)
		{
			pcha = channel_data_get(list[i].ch);
			pchb = channel_data_get(list[j].ch);
			if(pcha==NULL || pchb==NULL)continue;
			va= (list[i].used)?( (list[i].hard)?((first)?(1):(220)):(pcha->Ufsoc*2)):(0);  //应急充电在前,有且只有一个,之后的排末尾		(权重)
			vb= (list[j].used)?( (list[j].hard)?((first)?(1):(220)):(pchb->Ufsoc*2)):(0);  //之后，电量最大的依次排列                 (权重)
			if(va<vb)
			{
				Queue_Type t = list[i];//交换
				list[i] = list[j];
				list[j] = t;
			}
		}
		if(list[0].used && list[0].hard && first==FALSE)first=TRUE;
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
		pch = channel_data_get(l.ch);
		if(pch==NULL)continue;
		if(l.used && (hangall==FALSE))//有效 and 不挂起
		{
			if(charge_counter<CHANNEL_CHARGE_MAX)
			{	
				charge_counter++; //当前计数++
				l.charge=1;       //正在充电
				set_out5v();      //输出5V
				continue;
			}
		}			
		//不充电:未排到,挂起的
		reset_out5v();
		l.charge=0;
	}	
	return charge_counter;
}

/*计时*/
static void charge_timeout(void)
{
	static time_t t = 0;
	if(t==0)t=time(0)/1000;
	if( t != (time(0)/1000) )
	{
		t = time(0)/1000;
		//正常倒计时
		if(hangall==FALSE)
		{
			int ch=0;           
			for(;ch<CHANNEL_MAX;ch++) //选择前面的充电
			{
				if(l.used&&l.charge)    //充电计时,不充电不计时
				{
					if(l.charge_time!=0)
						l.charge_time--;
					else
					{
						l.used=0;//停止充电
					}
				}
			}		
		}
		//挂起倒计时
		else{
			if(hangtime>0)hangtime--;
		}	
	}
	
	//挂起超时，恢复倒计时
	if(hangtime==0)hangall=FALSE;
}

/*队列初始化*/
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
	inited=TRUE;
}

/*查找元素*/
static Queue_Type*request_channel_find(U8 channel)
{
	if(!inited)request_init();
	{
		int ch = 0;
		for(;ch<CHANNEL_MAX;ch++)
		{
			if(l.ch==channel)return &l;
		}
	}
	return NULL;
}

/*直接输出*/
static BOOL direct_charge(U8 ch,BOOL charged)
{
	Channel*pch = channel_data_get(ch);
	if(pch==NULL)return FALSE;
	if(charged==FALSE)reset_out5v();
	else set_out5v();
	return TRUE;
}
/*===================================================
                全局函数
====================================================*/

/*申请充电*/
BOOL request_charge_on(U8 ch,U32 seconds,BOOL hard)
{
	Queue_Type *qt = request_channel_find(ch);
	if(!inited)request_init();//未初始化，应该初始化
	if(qt==NULL)return FALSE;
  qt->hard=hard;            //hard=1:表示5秒or10min紧急充电
	qt->charge_time = seconds;//+(((time(0)%1000)>500)?1:0);//申请充电时间
	qt->used=1;               //有效标志 
	return TRUE;
}

/*中止充电*/
BOOL request_charge_off(U8 ch)
{
	if(!inited)request_init();
	Queue_Type *qt = request_channel_find(ch);
	qt->charge=qt->hard=qt->used=0;//复位有效标志
	qt->charge_time=0;
	{
		Channel*pch = channel_data_get(qt->ch);
		if(pch==NULL)return FALSE;
		/*-----------------------------------------
		*  如果是强制充电，对于单个仓道，不应该断电
		*  否则出现三角波
		------------------------------------------*/
		if(!is_force_charge_on(ch))
			reset_out5v();           //马上断电
	}
  return TRUE;
}
/*挂起一个充电*/
BOOL request_charge_hangup(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	Queue_Type *qt = request_channel_find(ch);
	if(pch==NULL)return FALSE;
	if(qt ==NULL)return FALSE;
	if(!inited)request_init();
	qt->used=0;
	reset_out5v();           //马上断电
	return TRUE;
}

/*恢复一个充电*/
BOOL request_charge_recovery(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	Queue_Type *qt = request_channel_find(ch);
	if(pch==NULL)return FALSE;
	if(qt ==NULL)return FALSE;
	if(!inited)request_init();
	qt->used=1;
	return TRUE;
}

/*挂起充电*/
BOOL request_charge_hangup_all(U32 seconds)
{
	if(!inited)request_init();
	{
		int ch;
		for(ch=1;ch<=CHANNEL_MAX;ch++)
		{
			Channel*pch = channel_data_get(ch);
			if(pch==NULL)continue;
			//if(!is_force_charge_on(ch))//不强制充电,就断电
			 reset_out5v();//禁止所有输出
		}
		hangall=TRUE;
		hangtime=seconds; //挂起，并倒计时
		return TRUE;
	}
}

/*充电调度器是否挂起?*/
BOOL ld_is_queue_hang(void)
{
	//挂起后，按强制充电流程
	return (BOOL)((system.enable==0)||((system.enable==1)&&(system.mode==1)));
}
/*===================================================
                充电调度任务
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(queue)
{
	PROCESS_BEGIN();
	while(1)
	{
		//可以充电
		if(system.enable==1)
		{
			//强制充电方式
		  if(system.mode==1)
			{
				int i = 0;
				for(;i<CHANNEL_MAX;i++)
				{
					direct_charge(i+1,(BOOL)system.chs[i]);//强制输出
				}
			}	
			//自由充电方式
			else{
			bubble_sort();          //排序
			charge_timeout();			  //计时
			charge_front();         //前面先充		
		 	}
		}
		
		//不充电
		else{
		  request_charge_hangup_all(0);//强制关断
		}
 	  os_delay(queue,100);   
	}
	PROCESS_END();
}

