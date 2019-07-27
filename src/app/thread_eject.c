#include "includes.h"
/*-----------------------------------------------------------
 //(到位开关无效 && 摆臂开关有效 && 读不到数据)==>认为是一个锁仓事件
 //弹仓>6次失败后,等待2小时后，重新开始，检测锁仓
 //弹仓后,eject 告警标志置位==>10分钟后清0
------------------------------------------------------------*/
static U8 _clec[CHANNEL_MAX];      //去抖计数
static U8 _cleec[CHANNEL_MAX];     //弹仓失败计数
static int _cle10min[CHANNEL_MAX];     //10分钟计时,秒
static int _cle2hour[CHANNEL_MAX];     //2小时计时,秒

#define clec _clec[i-1]
#define cleec _cleec[i-1]
#define cle10min _cle10min[i-1]
#define cle2hour _cle2hour[i-1]

void channel_lock_timer(int ms)
{
	U8 i = 1;
	static int t = 0;
	t+=ms;
	if(t>=10000)//10秒
	{
		t-=10000;
		for(;i<=CHANNEL_MAX;i++)
		{
			//10分钟，
			if(cle10min>0)
			{
				cle10min-=10;
				if(cle10min<=0)
				{
					Channel*pch = channel_data_get(i);
					cle10min=0;
					if(pch)pch->warn.eject=0;//事件标志清0					
				}
			}
			if(cle2hour>0){
				cle2hour-=10;
				if(cle2hour<=0)
				{
					clec=cleec=cle2hour=0;//重新开始
				}
			}
		}
	}
}

/*------  锁仓 -- 弹仓 -------*/
//运输过程，可能出现 摆臂开关 自锁
AUTOSTART_THREAD_WITH_TIMEOUT(eject)
{
	static Channel*pch;
	static U8 i = 0;
	PROCESS_BEGIN();
  while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			pch=channel_data_get(i);
			if( (isvalid_daowe()==0) && (isvalid_baibi()==1) && (is_readok()==0) && (cleec <EJECT_FAIL_TIMES) )//(到位开关无效 && 摆臂开关有效 && 读不到数据)
			{
				delayus(300);//去抖
				if( (isvalid_daowe()==0) && (isvalid_baibi()==1) && (is_readok()==0) )
				{
					clec++;
					if(clec>=2)
					{
						clec=0;
						request_charge_hangup_all(1); //挂起所有充电
						os_delay(eject,10);//延时10ms
						dian_ci_fa(pch,HIGH);//电磁阀动作
						os_delay(eject,500);//等待500ms
						dian_ci_fa(pch,LOW);//电磁阀空闲
						pch->warn.eject = 1;//事件位置1
						cle10min=600;       //事件位清0,计时10分钟后eject清0
            os_delay(eject,50); //延时
            if(isvalid_baibi()) //摆臂开关还有效表示没有弹仓成功
						{
							pch->error.motor = 1;//电磁阀故障
							cleec++;          //错误计数++
							if(cleec==EJECT_FAIL_TIMES)cle2hour=EJECT_FAIL_WAIT;//等待2小时
						}
						else 
						{
							cleec=0;	
							pch->error.motor = 0;//电磁阀故障清0
						}							
					}
				}
			}
		}
		
		//10秒钟检测一次 EJECT_INTERVAL==10000
		{
			static time_t last = 0;last=time(0)+EJECT_INTERVAL;          //上一次时间
			do{os_delay(eject,1000);}while( (time(0)-last)> 0x80000000 );//超时检测
		}
	}
	PROCESS_END();
}
