#include "includes.h"

/*===================================================
                充电宝读-状态机
			通过发送消息的方式，统一iic,ir读写接口
			50ms的长延时被状态机伪阻塞
====================================================*/
#define READ_TYPE_DATA_MAX  16
/*  命令 */
typedef enum{
	READ_TYPE_READ_ID,        //:读充电ID
	READ_READ_DATA,           //:读数据
	READ_READ_UNLOCK,         //:解锁05
	READ_READ_LOCK,           //:上锁06
	READ_UNLOCK_1HOUR,        //:解锁1小时07
	READ_OUTPUT,              //:读输出标志
}READ_TYPE_CMD;//充电宝命令


/*读写方式*/
typedef enum{
	READ_TYPE_MODE_IIC=0,
  READ_TYPE_MODE_IR,
}READ_TYPE_MODE;

/*读写配置*/
typedef struct{  
	READ_TYPE_MODE mode;  // 0:iic   1:红外
	//===========iic数据================//
	U8 sda;
	U8 scl;
	//===========红外===================//
  U8 ch;//1-n
	//===========命令===================//
	READ_TYPE_CMD cmd;
}Read_Type_Ctrl;

/*消息结构体*/
typedef struct{
	BOOL used;            //TRUE :有效
	BOOL start;           //FALSE:完成   TRUE:开始
	Read_Type_Ctrl ctrl;  //控制
	U8 data[READ_TYPE_DATA_MAX];            //数据缓冲
	FSM fsm;              //状态机
}Read_Type;

static Read_Type rts[CHANNEL_MAX * 2];
static void fsm_read(Read_Type*rt,FSM*fsm)
{
	fsm_time_set(time(0));
	
	Start(开始){
		
		if(rt->used==TRUE && rt->start==TRUE)
		{
			
		}
		
		
		//iic
		if(rt->ctrl.mode==READ_TYPE_MODE_IIC)
		{
		
		}
		else if(rt->ctrl.mode == READ_TYPE_MODE_IR)
		{
			
		}	
	}
	

}

/*===================================================
                充电宝读任务
====================================================*/
static struct etimer et_bao;
PROCESS(thread_bao, "通道任务");
AUTOSTART_PROCESSES(thread_bao);
PROCESS_THREAD(thread_bao, ev, data)  
{
	PROCESS_BEGIN();
  channel_data_init();//初始化仓道数据
	while(1)
	{
    U8 i=0;
		for(;i<CHANNEL_MAX*2;i++)
		{
			if(rts[i].used){fsm_read(rts+i,&(rts+i)->fsm);}//运行所有操作状态机
		}
		os_delay(et_bao,10);
	}
	PROCESS_END();
}

/*===================================================
                充电宝读对外接口
====================================================*/
/*读取一个充电宝的数据*/
BOOL ld_bao_read_start(Read_Type_Ctrl ctrl)
{

}
/*查询是否读成功, 成功就返回数据*/
BOOL ld_bao_read_isok(Read_Type_Ctrl ctrl,U8*dataout)
{
	
}

/*===================================================
                仓道任务
====================================================*/
static struct etimer et_channel;
PROCESS(thread_channel, "通道任务");
AUTOSTART_PROCESSES(thread_channel);
PROCESS_THREAD(thread_channel, ev, data)  
{
	PROCESS_BEGIN();
  channel_data_init();//初始化仓道数据
	while(1)
	{

	}

	PROCESS_END();
}


