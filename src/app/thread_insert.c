#include "includes.h"


/*===================================================
                充电流程
 严格按照 <通道给充电宝充电的逻辑.pdf> 流程控制								
====================================================*/
#define timeout(x)  (x<=0)
#define wait_timeout(condition,to,linenow)  end=to;line=linenow+1;case linenow+1: \
														                if((end>0) && (!(condition)) )return;
																												
/*-----------------------------
1.arg  0x99 开始

2.运行过程
		
	EXTI(外部中断)---->开始--->insert线程运行状态机(识别,充电，补充)
		
-------------------------------*/
static U8   _line[CHANNEL_MAX]={0};//当前状态
static U8   _last[CHANNEL_MAX]={0};//上一次状态          :counter
static int  _end[CHANNEL_MAX]={0}; //超时 ms             :wait_timeout
static int  _timeout[CHANNEL_MAX]={0};//秒 120秒电流计时 :ato
static BOOL _hangup[CHANNEL_MAX]={0};//计时挂起          :hang
static int  _btimeout[CHANNEL_MAX]={0};//补充计时        :bto
	
#define line _line[ch-1]
#define end _end[ch-1]
#define last _last[ch-1]
#define ato  _timeout[ch-1]
#define hang _hangup[ch-1]
#define counter _last[ch-1]
#define bto  _btimeout[ch-1]

//计时使用外部定时器,ms:中断时长
void charge_fms_timer(int ms)
{
	int i = 0;
	static int second = 0;
	BOOL is_second = FALSE;
	
	//秒计时
	second+=ms;
	if(second>1000)
	{	
		second-=1000;
		is_second=TRUE;
	}	
	
	for(;i<CHANNEL_MAX;i++)
	{
		if(_hangup[i])continue;
		if(_end[i]>0)_end[i]-=ms; //倒计时
		if(is_second && _timeout[i]>0)
			_timeout[i]--;
	}
}

void charge_fsm(U8 ch,void*arg)
{
	Channel*pch=channel_data_get(ch);	//仓道数据
	if(pch==NULL)return; 

	switch(line)
	{	
		//开始
		case 0:
						last=end=ato=hang=0;
					  if((int)arg==0x99){line=1;return;}
			break;
		//进入
		case 1:	if(isvalid_baibi()){line++; last=1;}return; 
		
		//识别/再识别
		case 2:
						wait_timeout(is_readok(), 3000, 2);//3秒内是否识别
						/*---------------能识别---------------------------------*/
						if(is_readok())
						{						
							if(pch->Ufsoc>0){line=20;return;} //电量>0===>停止充电
							else {
								line=10;
								request_charge_on(ch,600,TRUE);      //==>充电10分钟
								return;
							}
							
						/*---------------不能识别---------------------------------*/
					 }else{	
						if(last==1){
							line=4;
							request_charge_on(ch,5,TRUE);			//==>充电5秒			
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

		//停止充电==>充电7小时
		case 20:
						request_charge_on(ch,7*3600,FALSE);//充电7小时
						line=24;end=ato=counter=bto=0;return;
		break;
						
		//充电7小时
	  case 24:
						if(timeout(end) || pch->state.full_charge)
						{
							request_charge_off(ch);
							end=ato=counter=bto=0;
							line=26;	
						}
		break;
						
		//充电完成
		case 26:
					if(pch->Ufsoc<BUCHONG_STOP_UFSOC_MAX && pch->Ufsoc>BUCHONG_1HOUR_STOP_UFSOC_MAX)//85%-99%,充电3次，时间1小时
					{
						if(timeout(bto))//补充
						{
							bto=3600;counter++;
							if(counter>BUCHONG_1HOUR_TIMES)//3次后,充电结束
							{
								end=ato=counter=bto=0;
								request_charge_off(ch);
								line=30;
								return;
							}
							request_charge_on(ch,bto,FALSE);
						}
					}
					
					if(pch->Ufsoc<=BUCHONG_1HOUR_STOP_UFSOC_MAX)//<=85%,无限补充/3hour
					{
						counter=0;
						if(timeout(bto))
						{
							bto=3*3600;
							request_charge_on(ch,bto,FALSE);
						}
					}
					
					if(pch->Ufsoc>=BUCHONG_STOP_UFSOC_MAX)
					{
						end=ato=counter=bto=0;
						request_charge_off(ch);
						line=30;return;//充电完成
						
					}
		break;
					
		//充电结束
		case 30:				
		break;
	}
	
	  //无输出时，挂起计时
		if(line>=24)
		{
			if(isout5v()==0)
				hang=TRUE;
			else 
				hang=FALSE;
	  }
		
		//判断充电电流<100mA持续2min
		if( (pch->AverageCurrent<STOP_CURRENT_MAX) && (line>=24) && (pch->state.full_charge!=1))
		{
			if(ato==0)ato=time(0)+1000*STOP_CURRENT_TIMEOUT;
			else{
				if(timeout(ato))
					pch->state.full_charge=1;
			}
		}else ato=0;
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

