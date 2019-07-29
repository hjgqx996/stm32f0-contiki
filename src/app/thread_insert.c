#include "includes.h"


void charge_fsm(U8 ch,void*arg);
/*===================================================
                充电流程
 严格按照 <通道给充电宝充电的逻辑.pdf> 流程控制								
====================================================*/
#define timeout(x)  (x<=0)
#define wait_timeout(condition,to,linenow)  end=to;line=linenow+1;case linenow+1: \
    												                if((end>0) && (!(condition)) )return;
																						
//宏:申请一次充电充电时间为seconds秒,充电可以被挂起
#define request_charge_and_wait_timeout(seconds,hard,nextline)  \
			/*申请充电*/                  request_charge_on(ch,seconds,hard); \
			/*设置超时时间*/              end = time(0)+1000*seconds; \
			/*标志一下当前为申请充电状态*/request=TRUE;line=nextline;return
																						
/*上电检测，如果发现在有充电宝，按充电流程走*/
void recover_when_powerup(void)
{
	int i = 0;
	for(i=1;i<=CHANNEL_MAX;i++)
	{
		Channel*pch = channel_data_get(i);
		if(pch==NULL)continue;
		if( isvalid_baibi() )
		{
			charge_fsm(i,(void*)0x99);
		}
	}
}
/*-----------------------------
   状态机变量
-------------------------------*/
static U8   _line[CHANNEL_MAX]={0};//当前状态
static U8   _last[CHANNEL_MAX]={0};//上一次状态          :counter

static int  _end[CHANNEL_MAX]={0}; //超时 ms             :wait_timeout
static int  _timeout[CHANNEL_MAX]={0};//秒 120秒电流计时 :ato
static BOOL _hangup[CHANNEL_MAX]={FALSE};//计时挂起      :hang
static BOOL _request[CHANNEL_MAX]={FALSE};//申请充电     :request
static int  _btimeout[CHANNEL_MAX]={0};//补充计时        :bto
static int  _failcounter[CHANNEL_MAX]={0};     //充电宝不存在计数:failcounter
	
#define line    _line[ch-1]    //状态机当前行号
#define end     _end[ch-1]     //ms超时
#define last    _last[ch-1]    //上一次状态
#define ato     _timeout[ch-1] //120秒计时
#define hang    _hangup[ch-1]  //挂起:TRUE ==>停止计时
#define counter _last[ch-1]    // 1小时充电一次，充3次 的次数计数
#define bto  _btimeout[ch-1]   // 3小时计时 1小时计时用,秒
#define request _request[ch-1] //是否申请了充电
#define failcounter     _failcounter[ch-1]

static U8 buffer[16];
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
		if(end>0)end-=ms; //倒计时
		if(is_second)
		{
			if(ato)ato--;
			if(bto)bto--;
		}
	}
}

/*------------------------------------------------------------------------------------------------------
   充电流程
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
void charge_fsm(U8 ch,void*arg)
{
	int result = 0;
	extern BOOL is_system_in_return(U8 addr);
	Channel*pch=channel_data_get(ch);	      //仓道数据
	if(pch==NULL)return; 

	if((is_system_in_return(pch->addr)==TRUE) )return;//当前是归还仓道，不读(另有归还线程在读)
	if((int)arg==0x88){line=0;return;}                //复位

	/*-----------------------充电状态机-------------------------------------------*/
	switch(line)
	{	
		//开始(等待中断触发/上电触发==>arg==0x99)
		case 0:
						last=end=ato=hang=0;
					  if((int)arg==0x99){ pch->first_insert=TRUE; channel_clear(ch); line=1;}return;   //中断触发,上电触发(0x99)
						
		//进入
		case 1:	
						if(isvalid_baibi()){
								line++; last=1;  //识别一次//摆臂开关检测到,下一步
						}
						else 
							return;//等待摆臂开关
		
		//识别/再识别
		case 2:
						/*---------------能识别---------------------------------*/
						                  result = channel_read(pch,RC_READ_ID,buffer,500,TRUE);    if(result==-1)return; //红外拉高，忙，不读,识别一次
		        if(result==FALSE) result = channel_read(pch,RC_READ_ID,buffer,500,TRUE);    if(result==-1)return; //红外拉高，忙，不读,再识别一次
		        if(result==TRUE) result = channel_read(pch,RC_READ_DATA,buffer,600,TRUE);   if(result==-1)return; //红外拉高，忙，不读
		        if(result==TRUE)
						{			
							pch->first_insert=FALSE;
              pch->state.read_ok=1;							
							if(pch->Ufsoc>0){line=20;return;}  //电量>0===>停止充电
							else {                             //电为0 ===>申请充电10分钟,跳到line=10
								request_charge_and_wait_timeout(600,TRUE,10);
							}
						/*---------------不能识别---------------------------------*/
					 }else{	
            pch->first_insert=FALSE;						 
						if(last==1){                        //从1跳来的===>充电5秒(跳到line=4)
								request_charge_and_wait_timeout(5,TRUE,4);
						}				
						else if(last==4){ line=1;}          //充电5秒无法识别，复位                  
					 }
					 return;
					 
		//充电5秒
		case 4:
						if(timeout(end))
						{
							line=2;last=4;      //==》再识别一次
						}return;
						
		//充电10分钟
		case 10:
			      if(timeout(end) || (pch->Ufsoc>0))
						{
							request=FALSE;request_charge_off(ch); line=20;//超时 or 电量>0 ===>停止充电
						}return;
						
		//停止充电==>充电7小时
		case 20:
			      ato=counter=bto=0;
		        request_charge_and_wait_timeout(7*3600,FALSE,24);//充电7小时,跳到line=24

		//充电7小时
	  case 24:
						if(timeout(end) || pch->state.full_charge)//充电满 or 超时
						{
							request_charge_off(ch);//断输出
							end=ato=counter=bto=0;line=26;//充电完成(补充阶段) 	
						}
		break;
						
		//充电完成(补充阶段)
		case 26:
					if( (pch->Ufsoc<BUCHONG_STOP_UFSOC_MAX) && (pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX))//85%-99%,充电3次，时间1小时
					{
						if(timeout(bto))//补充
						{
							bto=3600;counter++;//超时时间1小时
							if(counter>BUCHONG_1HOUR_TIMES)//3次后,充电结束
							{
								end=ato=counter=bto=0;
								request_charge_off(ch);//申请断电
								request=FALSE;
								line=30;//充电结束
								return;
							}
							request_charge_on(ch,bto,FALSE);//申请充电
							request=TRUE;
						}
					}
					
					else if(pch->Ufsoc<=BUCHONG_1HOUR_STOP_UFSOC_MAX)//<=85%,无限补充/3hour
					{
						counter=0;
						if(timeout(bto))
						{
							bto=3*3600;
							request_charge_on(ch,bto,FALSE);
							request=TRUE;
						}
					}
					
					else if(pch->Ufsoc>=BUCHONG_STOP_UFSOC_MAX)
					{
						end=ato=counter=bto=0;
						request_charge_off(ch);
						line=30;request=FALSE;
						return;//充电完成
						
					}
		break;
					
		//充电结束
		case 30:
			    request_charge_off(ch);
					request=FALSE;
		break;
		default:break;
	}
	 
		//申请输出，无输出时，挂起计时
		if( ((isout5v()==0) && (request==TRUE)) )
			hang=TRUE;
		else 
			hang=FALSE;
		
		//充电宝读不到,复位，从头开始
		if( isvalid_baibi() && is_readerr() )
		{
			request_charge_off(ch);
			line=1;
			return;
		}
		
		//判断充电电流<100mA持续2min
		if( (pch->AverageCurrent<STOP_CURRENT_MAX) && (line>=24) && (pch->state.charging))
		{
			if(ato==0)
				ato=STOP_CURRENT_TIMEOUT;//开始倒计时
		}
		if( pch->AverageCurrent>=STOP_CURRENT_MAX)
			ato = 0;//电流大了，超时复位
		
		if(ato==1 || ato==2)//最后一秒断超时
		{
			pch->state.full_charge=1;//已经充满
			request_charge_off(ch);  //申请断电
			request=FALSE; 
			ato=0;
		}
}

///*===================================================
//                全局函数
//====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(insert)
{
	static int i= 0;
	PROCESS_BEGIN();
	
	os_delay(insert,500);
	recover_when_powerup();//上电检测到已经有宝，按插入流程充电
	
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			charge_fsm(i,NULL);
		}
		os_delay(insert,40);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

