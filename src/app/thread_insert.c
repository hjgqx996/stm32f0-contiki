#include "contiki.h"
#include "types.h"
#include "channel.h"
/*===================================================
                本地函数
====================================================*/
/*申请充电，并挂起自己状态*/
#define hang_up_until_charge(ch,state) 

/*检测到充电，恢复挂起的状态*/
#define recovery_state(ch)  




/*充电宝进入通道状态机*/
static void charge_fsm(U8 channel,void*arg)
{

	//channel数据
	FSM*fsm;
	Channel*ch;
	Start(检测充电宝进入){
		//检测条件:     检测成功->识别充电宝
				
	}
	
	State(识别充电宝)
	{
    //能识别-->停止充电
		//不能识别->充电5秒
	}
	
	State(充电5秒)
	{
	
		//申请充电
		
		//等待5秒后，停止充电-->再一次识别充电宝
		
	}
	State(再一次识别充电宝)
	{
	  //能识别-->电量是否为0
		
		//不能识别-->上报数据 并跳到 检测充电宝进入 重新开始
	
	}
	
	State(电量是否为0)
	{
		//电量<=0-->申请充电
		
		//电量 >0-->停止充电
		
	}
	
	State(充电10分钟)
	{
	  //申请充电
		
		//电量 >0 或 超时10分钟-->停止充电

	}
	
	State(停止充电)
	{
	  //排队充电
		goto 充电7小时;
	}
	
	State(充电7小时)
	{
		
		//申请充电
		
		//已经充电
		if(queue_isok(ch)==1)
		{
			//输出5V
		
			//已经充电7小时->补偿充电
			
			//电流<200mA,待续120秒->补偿充电
			
		}else{
			
		  //挂起计时
			
			//不输出5V
		}
		
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
const char*vd="kaihua.yang";
static process_event_t pet={0xF5};
static struct etimer et_insert;
PROCESS(insert_thread, "进入流程");
PROCESS_THREAD(insert_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		PROCESS_NAME(comm_thread);
		pet++;
    process_post(&comm_thread,pet,(void*)vd);
		os_delay(et_insert,10);
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(insert_thread);