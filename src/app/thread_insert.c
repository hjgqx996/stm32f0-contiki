#include "includes.h"


/*===================================================
                充电流程
 严格按照 <通道给充电宝充电的逻辑.pdf> 流程控制								
====================================================*/
/*-----------------------------
1.arg  0xFF (复位状态机)
       0xFE (挂起当前状态)
			 0xFD (恢复挂起的状态)
2.运行过程
		
	EXTI(外部中断)---->复位状态机--->insert线程运行状态机(识别,充电，补充)
		
-------------------------------*/
static FSM insert_fsm_data[CHANNEL_MAX]; 					//状态机变量:私有
void charge_fsm(U8 ch,void*arg)
{
	FSM*fsm =&insert_fsm_data[ch-1];
	Channel*pch=channel_data_get(ch);	//仓道数据
	fsm_time_set(time(0));            //状态机时间
	if(pch==NULL)return; 
	
	Start(){
			memset(fsm,0,sizeof(FSM));
			if(isvalid_baibi())goto 识别;
	}
	State(识别)
	{
		//等待读完成(识别在thread_channel中实现)
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
		
//		//已经充电
//		if(queue_isok(ch)==1)
//		{
//			//输出5V
//		
//			//已经充电7小时->补偿充电
//			
//			//电流<200mA,待续120秒->补偿充电
//			
//		}else{
//			
//		  //挂起计时
//			
//			//不输出5V
//		}
		
	}
	
	State(补偿充电)
	{
	  //电量>95%:充电完成
		//电量>=85%,<=95%:1小时补充1次，3次机会
		//电量<85%,3小时补充一次，无限机会
	
	}
  Default()
	
}

/*===================================================
                全局函数
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(insert)
{
	static int i= 0;
	PROCESS_BEGIN();
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			charge_fsm(i,NULL);
		}
		os_delay(insert,50);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

