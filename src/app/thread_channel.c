#include "includes.h"


///*===================================================
//               宏,类型，变量
//====================================================*/
static FSM rdfsm[CHANNEL_MAX];
///*===================================================
//                私有函数
//====================================================*/
/*--------------------------------------------------------------

*	保存数据*
*	[0] 版本号 [1] 电量 [2] 温度 [3] 故障码 [4-5] 循环次数 [6-7] 容量 [8-9] 电芯电压 [10-11] 电流 (低位在前)
*---------------------------------------------------------------
*/
void save_data(Channel*ch,U8*data)
{
	if(ch==NULL||data==NULL)return;
	ch->Ver					= data[0];
	ch->Ufsoc				= data[1];
	ch->Temperature			= data[2];
	ch->CycleCount			= (((U16)data[5])<<8)|(data[4]);
	ch->RemainingCapacity	= (((U16)data[7])<<8)|(data[6]);
	ch->Voltage				= (((U16)data[9])<<8)|(data[8]);
	ch->AverageCurrent		= (((U16)data[11])<<8)|(data[10]);
}



//约定 i:尝试次数 j:超时计数 
#define read(cmd,times,timeout,sucess,fail,error) \
	/*数据复位*/fsm->i=0;fsm->j=timeout/10; \
	/*尝试3次*/		while(fsm->i++<times) \
								{ \
									do{ \
	/*读数据*/					err=channel_read(pch,cmd,dataout); \
	/*完成跳出*/				if(err>=2)break; \
	/*等待完成*/				waitms(10); \
	/*本次超时*/			}while(fsm->j--!=0); \
	/*成功*/				if(err==2) sucess \
	/*失败*/				else fail \
	/*几次都失败*/			if(fsm->i>=3)error \
								}


/*--------------------------------------------------------
1.对所有通道检查，看充电宝是否有效
2.有效的充电宝执行以下操作:
  (1) 读ID 
  (2) 读数据
  (3) 读输出标志
  (4) 6代宝以上,非租借条件下，应该 加密
---------------------------------------------------------*/
static void read_data_fsm(Channel*pch,U8 ch)
{
	FSM*fsm = &rdfsm[ch-1];
	int err=0;
	U8 dataout[13];

	fsm_time_set(time(0));
	
	if(pch->readerr>BAO_READ_ERROR_RETYR_TIMES)pch->readok=0;
	if(pch->readok>=2){pch->readok=2;pch->readok=1;}
	
	/*摆臂开关有效可以读数据*/
	if(isvalid_baibi())
	{
		Start(){
			if(isvalid_baibi()){memset(fsm,0,sizeof(FSM));goto id;}
		}
		//读id
		State(id)
		{
			read(RC_READ_ID,2,1000,             //读id,2次----超时1s
					{
						memcpy(pch->id,dataout,10);   //成功
						pch->readok++;goto data;
					},
					{pch->readerr++;},              //失败
					{fsm->line=0;return;});               //错误
		}

		//读数据
		State(data)
		{
		  /*------读id------2次----超时1s---成功保存数据------------------------------------------失败次数++-------- 都失败复位状态机*/
			read(RC_READ_DATA ,2     ,1000  ,{save_data(pch,dataout);pch->readok++; goto lock678;},{pch->readerr++;},{fsm->line=0;return;});
		}

		//加密 6,7,8代宝(非租借条件下)
		State(lock678)
		{
			if(is_ver_6() || is_ver_7())
			{
				if(pch->bao_output!=0x06)
				{
					/*------读id--1次--超时1s---成功保存数据-------------------失败次数++-------- 都失败复位状态机*/
					read(RC_LOCK ,1     ,1000  ,{pch->bao_output=dataout[0];},{pch->readerr++;},{fsm->line=0;return;});
				}
			}
			
		  //复位状态机，从头开始
			memset(fsm,0,sizeof(FSM));
			return;
		}
		Default()
	}

	/*摆臂开关无效数据清0*/
	else {
		delayms(1);
		if(!isvalid_baibi())
			channel_data_clear(ch);
	}
}


/*===================================================
						仓道任务: 读数据
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	U8 i = 0;
	static time_t t = 0;
	
	PROCESS_BEGIN();          
	while(1)
	{
		t=time(0);
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			Channel*pch = channel_data_get(i);
				if(pch==NULL)continue;                        
			read_data_fsm(pch,i);
		}
		
		//读失败，不延时    读成功：最小延时2.8秒
		if(t!=0) { t= (BAO_READ_DATA_MAX_MS -(time(0)>t?time(0)-t:(0xFFFFFFFF-t+time(0))))%BAO_READ_DATA_MAX_MS;}
		if(t>0){os_delay(channel,t);}
		ld_iwdg_reload();
		
	}
	PROCESS_END();
}

/*===================================================
						仓道任务: 实时检测仓道状态
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel_state)
{
	static U8 i = 0;
	PROCESS_BEGIN();
	while(1)
	{
		for(i=0;i<CHANNEL_MAX;i++)
		{
		/*=====================状态位检测=========================*/
							channel_state_check(i+1);
		/*=====================告警位检测=========================*/
							channel_warn_check(i+1);
		/*=====================错误位检测=========================*/ 
							channel_error_check(i+1);
		}
		os_delay(channel_state,100);
		ld_iwdg_reload();
	}
	PROCESS_END();
}