#include "contiki.h"
#include "types.h"

/*===================================================
                本地函数
====================================================*/
/*充电宝进入通道状态机*/
static void charge_fsm(U8 channel,void*arg)
{

	//channel数据
	FSM*fsm;
	Start(检测充电宝进入){
		//检测条件:
				
	}
	
	State(识别充电宝)
	{
    //能识别-->冒泡优先
		//不能识别->加电5秒
	}
	
	State(充电5秒)
	{
	
		//充电5秒
		
		//再一次识别充电宝:能识别->电量是否为0    不能识别:弹出充电宝,复位数据->检测充电宝进入
	
	}
	
	State(电量是否为0)
	{
		//电量<=0，强制充电10分钟
		//电量 >0,停止充电
	}
	
	State(充电10分钟)
	{
	  //电量>0，停止充电
		//10分钟超时,停止充电
	}
	
	State(停止充电)
	{
	  //直接进入冒泡，排队充电
		goto 冒泡;
	}
	
	State(冒泡)
	{
	   //冒泡选择充电通道
	   
		 //通道被选择开始充电
	}
	State(充电7小时内)
	{
	  //电流<200mA,待续120秒->补偿充电
		
		//已经充电7小时->补偿充电
	}
	State(补偿充电)
	{
	  //电量>95%:充电完成
		//电量>=85%,<=95%:1小时补充1次，3次机会
		//电量<85%,3小时补充一次，无限机会
	
	}
  default:{}}
}




/*===================================================
                全局函数
====================================================*/
static struct etimer et_insert;
PROCESS(insert_thread, "进入流程");
PROCESS_THREAD(insert_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
	   etimer_set(&et_insert, CLOCK_SECOND);              // etimer溢出时间为5s
       PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_insert));  // 等待定时器溢出
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(insert_thread);