#include "includes.h"


extern BOOL is_system_in_return(U8 addr);
/*===================================================
							宏操作
====================================================*/
//是否超时
#define timeout(x)  (x<=0)   // is time out
																				
//宏:申请一次充电充电时间为seconds秒,充电可以被挂起
#define request_charge_and_wait_timeout(seconds,hard,nextline)  \
			/*申请充电*/                  request_charge_on(ch,seconds,hard); \
			/*设置超时时间*/              to = time(0)+1000*seconds; \
			/*标志一下当前为申请充电状态*/request=TRUE;goto nextline

//温度挂起条件
//电量>=50% 温度>50度,挂起充电
//电量<50%  温度>55度,挂起充电
#define is_temperature_high()   ( (pch->Ufsoc>=50&&pch->Temperature>50) ||(pch->Ufsoc <50&&pch->Temperature>55)	)
/*===================================================
						  私有变量
====================================================*/
static U16   _line[CHANNEL_MAX]={0};       //当前状态
static U8   _last[CHANNEL_MAX]={0};       //上一次状态          
static int  _to[CHANNEL_MAX]={0};         //timeout超时           
static int  _s120[CHANNEL_MAX]={0};       //秒 120秒电流计时 
static BOOL _hang[CHANNEL_MAX]={FALSE};   //计时挂起     
static BOOL _request[CHANNEL_MAX]={FALSE};//申请充电    
static BOOL _highTemp[CHANNEL_MAX]={FALSE};//高温标志
static int  _1hour[CHANNEL_MAX]={0};      //1小时补充计时
static U8   _1hourcount[CHANNEL_MAX]={0}; //1小时计数
static int  _3hour[CHANNEL_MAX]={0};      //3小时补充计时
static U8   buffer[16];                   //读充电宝时数据缓存

#define line     _line[ch-1]      //状态机当前行号
#define to       _to[ch-1]        //ms超时
#define last     _last[ch-1]      //上一次状态
#define s120     _s120[ch-1]      //120秒计时
#define hang    _hang[ch-1]       //挂起:TRUE ==>停止计时
#define counter _1hourcount[ch-1] // 1小时充电一次，充3次 的次数计数
#define hour1   _1hour[ch-1]      // 1小时充电计时
#define hour3   _3hour[ch-1]      // 3小时充电计时
#define request _request[ch-1]    //是否申请了充电
#define HTemp   _highTemp[ch-1]   //高温挂起标志

/*===================================================
			上电检测，如果发现在有充电宝，按充电流程走
====================================================*/																						
void recover_when_powerup(void)
{
	int trytimes = 0;
	while(trytimes < CHECK_TIMES_POWER_UP)
	{
		int i = 0;
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			Channel*pch = channel_data_get(i);
			if(pch==NULL)continue;
			if( isvalid_baibi() )
			{ 
				void fsm_charge(U8 ch,int arg);
				if(pch->state.read_ok==0 || pch->state.read_from_ir==1 || pch->error.thimble==1 )// 上电自检，如果iic失败，尝试4次
				{		
					if(pch->state.read_from_ir==1)delayms(250);
					fsm_charge(i,0x99);
					fsm_charge(i,NULL);
					fsm_charge(i,NULL);
					fsm_charge(i,NULL);
					ld_iwdg_reload();
				}
			}
		}
		trytimes++;
	}
}
/*===================================================
						  状态机计时函数
====================================================*/
//计时使用外部定时器,ms:中断时长
void charge_fms_timer(int ms)
{
	int ch = 1;
	static int second = 0;
	BOOL is_second = FALSE;
	
	//秒计时
	second+=ms;
	if(second>1000)
	{	
		second-=1000;
		is_second=TRUE;
	}	
	
	for(ch=1;ch<=CHANNEL_MAX;ch++)
	{
		if(hang)continue;
		if(to>0)to-=ms;  //倒计时
		if(is_second)
		{
			if(s120 >0)s120--;  //120秒计时
			if(hour1>0)hour1--;//1小时计时
			if(hour3>0)hour3--;//3小时计时
		}
	}
}

/*------------------------------------------------------------------------------------------------------
   充电流程 严格按照 <通道给充电宝充电的逻辑.pdf> 流程控制		
上电/充电宝exti=======>进入===(摆臂无效)=> 复位
                        +
                      (有效)                                                                        __
                        +======>等待3秒===>识别===(yes)==>电量大于0===(yes)=======================>|  |
                                            +                 |                                    |  |
                     (返回)归还通道<--------+                 |                                    |  |
                                           (no)===>y充电5秒===|=>识别==(yes)===>电量大于0==(yes)==>|停|
                                                              +                      +             |止|
                                                             (no)===>充电10分钟<====(no)           |充|
                                                                          +                        |电|
                                                                   (电量>0 or 超时)===============>|__|
                                                                                                     |
        +---(no)--->充电7小时<-----------------------------------------------------------------------+ 
        +               +
        +               +
         -------(电流<100,120秒)----(yes)--> 补充-->(85-99,1小时补充1次，补充3次)-->(<85% 3小时一次无限补充)-->(>=99,任务完成)
-------------------------------------------------------------------------------------------------------*/
#define start()           switch(line){case 0:
#define state(name)       break;name:line=__LINE__;return;case __LINE__:
#define defaultx()        break;default:{line=0;}break;}
void fsm_charge(U8 ch,int arg)
{
	int result = 0;
	Channel*pch=channel_data_get(ch);if(pch==NULL)return;//仓道数据

	if((is_system_in_return(pch->addr)==TRUE) )return;   //当前是归还仓道，不读(另有归还线程在读)
	if(arg==0x88){line=0;request_charge_off(ch);return;} //复位,arg=0x88,不充电
  if(arg==0x99){if(pch->state.read_ok)return; pch->first_insert=TRUE; goto entry;}   //中断触发,上电触发(0x99),清数据,跳到line=1
	if(arg==0x87){if(line==0) goto stop_charge;else return;} //充电宝插入,无法识别, 但 后来 可以识别了==>直接跳到(停止充电)stop_charge;
	/*-----------------------充电状态机-------------------------------------------*/
	//开始(等待中断触发/上电触发==>arg==0x99)
	start()
	{
		to=s120=hour1=hour3=counter=0; hang=FALSE;
		return;
	}
	
	/*================================状态:进入(有充电宝进入/上电)==================================================*/
	state(entry)
	{
		if(isvalid_baibi()){last =1;goto identify;}//用last=1标记一下，我是一开始进来的
		else 
			return;//等待摆臂开关
	}
	
	/*================================状态:识别/5秒后再识别=========================================================*/
	state(identify)
	{
				/*---------------能识别---------------------------------*/
													                result = channel_read(pch,RC_READ_ID,buffer,550,TRUE);                    	
				if(result==FALSE) {  delayms(100);result = channel_read(pch,RC_READ_ID,buffer,550,TRUE);} 
				if(result==TRUE)  {  
														result = channel_read(pch,RC_READ_DATA,buffer,650,TRUE);
					 if(result==FALSE){delayms(100);result = channel_read(pch,RC_READ_DATA,buffer,650,TRUE);}
				}
				
				if(result==TRUE)
				{			
					pch->first_insert=FALSE;
					pch->state.read_ok=1;							
					if(pch->Ufsoc>0){goto stop_charge;}  //电量>0===>停止充电
					else 
					{                                  
						request_charge_and_wait_timeout(POWERUP_TIME_0_UfSOC,TRUE,charge_10_min);//电为0 ===>申请充电10分钟
					}
				/*---------------不能识别---------------------------------*/
			 }else{	
					pch->first_insert=FALSE;						 
					if(last==1)
					{                        
							request_charge_and_wait_timeout(POWERUP_TIME_5_SECONDS,TRUE,charge_5_second);//===>充电5秒
					}				
					else{ line=0;}//充电5秒无法识别==>复位                
			 }
			 return;		
	}
	
	/*================================状态:等待充电5秒==============================================================*/
	state(charge_5_second)
	{
		if(timeout(to)){last=4;goto identify;}//用last=4标记一下，我是5秒充电过来的
		return;
	}
	
	/*================================状态:等待充电10分钟============================================================*/
	state(charge_10_min)
	{
		if(timeout(to) || (pch->Ufsoc>0))//超时 or 电量>0 ===>停止充电
		{
			request=FALSE;
			request_charge_off(ch);
			goto stop_charge;
		}return;
	}
	
	/*================================状态:充电任务完成==============================================================*/
	state(charge_complete)
	{	
		hour1=hour3=counter=s120=0;
		request_charge_off(ch);
		request=FALSE;
		if(pch->Ufsoc<=BUCHONG_STOP_UFSOC_MAX)goto recharge;//当电量再一次降低时==>重新补充
	}
	
	/*================================状态:停止充电==================================================================*/
	state(stop_charge)
	{
		s120=counter=hour1=hour3=0;
		request_charge_and_wait_timeout(7*3600,FALSE,charge_7_hours);//==>充电7小时
	}
	
	
	/*================================状态:7小时充电过程=============================================================*/
	state(charge_7_hours)
	{
		if(timeout(to) || pch->state.full_charge)//充电满 or 超时
		{
			request_charge_off(ch);       //断输出
			to=s120=counter=hour1=hour3=0;//充电完成(补充阶段)
			goto recharge;
		}		
	}
	
	/*================================状态:补充(检测是否补充)========================================================*/
	state(recharge)
	{
		request_charge_off(ch);       //断输出,对于强制输出,不应断电
		request=FALSE;                
		if( (pch->Ufsoc<=BUCHONG_STOP_UFSOC_MAX) && (pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX) )//85%-99%,充电3次，时间1小时
		{
			hour1=3600;
			goto recharge_3_times;
		}
		
		if(pch->Ufsoc<=BUCHONG_1HOUR_STOP_UFSOC_MAX)//<=85%,无限补充/3hour
		{
			hour3=3600*3;
			goto recharge_all_time;
		}
		
		if(pch->Ufsoc>BUCHONG_STOP_UFSOC_MAX)//==100%,充电完成
			goto charge_complete;
	}
	
	/*================================状态:每隔1小时 3次 补充=========================================================*/
	//85%-99% 补充3次,当充电完成后，计数清0
	state(recharge_3_times)
	{
		if(counter<=3)
	  {//只充3次
			if(timeout(hour1))//超时补充
			{
				counter++;
				hour1=3600;
				s120=0;
				request=TRUE;
				request_charge_on(ch,3600,FALSE);//申请充电
			}		
		}//3次补充，都没有超过99%==>充电完成
		else{
			hour1=s120=0;
			request=FALSE;
			request_charge_off(ch);
			goto charge_complete;
		}//申请断电

		//电量不在85%-99%,重新检测是否补充
		if(!((pch->Ufsoc<=BUCHONG_STOP_UFSOC_MAX) && (pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX)))
		{
			hour1=s120=counter=0;//又可以重新3次补充了
			goto recharge;
		}
	}
	/*================================状态:每隔3小时无限补充=========================================================*/
	state(recharge_all_time)
	{
		if(timeout(hour3))
		{
			hour3=3600*3;
			request=TRUE;s120=0;
			request_charge_on(ch,3600,FALSE);//申请充电
		}
		//电量>85%,退出无限补充
	  if(pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX)
		{
		  hour3=s120=0;
			goto recharge;
		}
	}
	defaultx()
	
	
	/*================================申请输出，无输出时，挂起计时=====================================================*/
	if( ((isout5v()==0) && (request==TRUE)) )
		hang=TRUE;
	else 
		hang=FALSE;
	
	//充电宝读不到,复位，从头开始
	if( isvalid_baibi() && is_readerr() )
	{
		request_charge_off(ch);
		line=0;
		return;
	}
	/*================================温度过高,挂起计时,等待温度降低,恢复充电===========================================*/
	if(is_temperature_high())
	{
		if(request==TRUE && HTemp==FALSE)
		{
				hang=TRUE;                //挂起
				HTemp=TRUE;               //高温标志
				request_charge_hangup(ch);//立即断电
		}
	}else{
		if(request==TRUE && HTemp==TRUE)
			request_charge_recovery(ch);//恢复排队
		HTemp=FALSE;                  //清高温标志
	}
	
  /*================================判断充电电流<100mA持续2min,断电===================================================*/		
	if( (pch->AverageCurrent<STOP_CURRENT_MAX) && (line>=200) && (pch->state.charging) && (request==TRUE))//出现电流<100mA
	{
		if(s120==0)
			s120=STOP_CURRENT_TIMEOUT+10;            //开始倒计时
	}
	if( pch->AverageCurrent>=STOP_CURRENT_MAX)//出现电流>=100mA,计时复位
		s120 = 0;
	
	if(s120 <=10 && s120 >0 )                   //最后一秒判断超时
	{
		pch->state.full_charge=1;              //已经充满
		request_charge_off(ch);                //申请断电
		request=FALSE; 
		s120=0;
	}
}

///*===================================================
//                全局函数
//====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(insert)
{
	static int i= 0;
	PROCESS_BEGIN();
	recover_when_powerup();            //上电检测到已经有宝，按插入流程充电
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			fsm_charge(i,NULL);           //充电状态机流程
			ld_iwdg_reload();             //喂狗
		}
		channel_data_map_init();        //2019-9-19: 端口配置初始化:长时间运行时不排除加载到内存中的端口配置出现问题,引起所有通道不正常工作
		                                //最好的办法是，检测并初始化配置，减小程序风险
		os_delay(insert,200);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

