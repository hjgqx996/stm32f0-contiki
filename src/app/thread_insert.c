#include "includes.h"


/*===================================================
                充电流程
 严格按照 <通道给充电宝充电的逻辑.pdf> 流程控制								
====================================================*/
#define timeout()  ((time(0)-end)<0x80000000)
#define wait_timeout(condition,to,linenow)  end=time(0)+to;line=linenow+1;case linenow+1: \
														if(((time(0)-end)>0x80000000) && (!(condition)) )return;
																												
/*-----------------------------
1.arg  0xFF (复位状态机)
       0xFE (挂起当前状态)
			 0xFD (恢复挂起的状态)
2.运行过程
		
	EXTI(外部中断)---->复位状态机--->insert线程运行状态机(识别,充电，补充)
		
-------------------------------*/
static U8 _line[CHANNEL_MAX]={0};//当前状态
static U8 _last[CHANNEL_MAX]={0};//上一次状态
static U16 _end[CHANNEL_MAX]={0}; //超时

#define line _line[ch-1]
#define end _end[ch-1]
#define last _last[ch-1]
void charge_fsm(U8 ch,void*arg)
{

	Channel*pch=channel_data_get(ch);	//仓道数据
	if(pch==NULL)return; 
	
	
	//状态挂起
	
	
	switch(line)
	{	
		//开始
		case 0:
						last=end=0;
			break;
		//进入
		case 1:	if(isvalid_baibi()){line++; last=1;}break; 
		
		//识别/再识别
		case 2:
						wait_timeout(is_readok(), 10000, 2);//10秒内是否识别
						/*---------------能识别---------------------------------*/
						if(is_readok())
						{						
							if(pch->Ufsoc>0){line=20;return;} //电量>0===>停止充电
							else {
								line=10;
								request_charge_on(ch,600);      //==>充电10分钟
								return;
							}
							
						/*---------------不能识别---------------------------------*/
					 }else{	
						if(last==1){
							line=4;
							request_charge_on(ch,5);			//==>充电5秒			
						}else if(last==4){
							line=0;                       //充电5秒无法识别，复位
						}
						return;
					 }
		break;

		//充电5秒
		case 4:
						wait_timeout(isout5v(), 7*3600*1000, 4);//7小时内是否输出
						
						if(isout5v())//有输出
						{
							wait_timeout(0,5000, 6);//等待5秒
							line=2;last=4;return;   //==》再识别一次
						}
						else{line=0;return;}//无输出//复位
						break;
			 
						
		//充电10分钟
		case 10:
						wait_timeout(isout5v(), 7*3600*1000, 10);//7小时内是否输出					
						if(isout5v())//有输出
						{
							wait_timeout((pch->Ufsoc>0),600*1000, 12);  //10分钟电量>0
							line=20;
							return;
						}
						//无输出
						else{line=20;return;}     //无输出==>停止充电
						break;						

		//停止充电
		case 20:break;
		
		break;
	
	}
	
	
	
	
	
//	Start(){
//			memset(fsm,0,sizeof(FSM));
//			if(isvalid_baibi())goto 识别;
//	}
//	State(识别)
//	{
//		wait_timeout(is_readok(),2000,10,{goto充电7小时;},{goto 充电5秒;});//等待读,读数据在 thread_channel中实现
//	}
//	
//	State(充电5秒)
//	{
//		request_charge_on(ch,5);
//		wait_timeout(isout5v(),25200000,1000,{break;},{fsm->line=0;return;});//等待7小时直到可以充电,超时从头开始
//		waitms(5000);
//		goto 再一次识别充电宝;
//	}
//	State(再一次识别充电宝)
//	{
//		wait_timeout(is_readok(),2000,10,
//		{
//			if(pch->Ufsoc==0)goto 充电10分钟;//电量为0充电10分钟
//			if(pch->Ufsoc>0)goto 停止;
//		},{goto 充电10分钟;}); 
//	}
//	State(充电10分钟)
//	{
//		request_charge_on(ch,600);
//		wait_timeout(isout5v(),25200000,1000,{break;},{fsm->line=0;return;});//等待7小时直到可以充电,超时从头开始
//		wait_timeout((pch->Ufsoc>0),600000,1000,{goto 停止;},{goto 停止;});  //充电10分钟直到电量>0
//	}
//	State(停止)
//	{
//		
//	}
//	State(充电7小时)
//	{
//		
//		//申请充电
//		
////		//已经充电
////		if(queue_isok(ch)==1)
////		{
////			//输出5V
////		
////			//已经充电7小时->补偿充电
////			
////			//电流<200mA,待续120秒->补偿充电
////			
////		}else{
////			
////		  //挂起计时
////			
////			//不输出5V
////		}
//		
//	}
//	
//	State(补偿充电)
//	{
//	  //电量>95%:充电完成
//		//电量>=85%,<=95%:1小时补充1次，3次机会
//		//电量<85%,3小时补充一次，无限机会
//	
//	}
//  Default()
	
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

