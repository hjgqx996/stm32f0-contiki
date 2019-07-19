#include "includes.h"


///*===================================================
//               宏
//====================================================*/
#define output()         (buf[0])

#define channel_read_start_wait_end(cmd,sucess,to) \
          /*初始化数据*/   error=0;timeoutcounter=to/10; \
					/*开始读数据*/   do{error=channel_read(pch,cmd,buffer); \
					/*延时10ms  */      os_delay(channel,10);timeoutcounter--; \
					/*读取完成或超时*/}while(error<2&&timeoutcounter>0);if(timeoutcounter<=0)error=4; \
															do{ \
					/*运行成功时执行*/       if(error==2){sucess; \
																		pch->readok++; \
					/*连续2次成功,就正常*/    if(pch->readok>2)pch->readok=2; \
                                    pch->readerr=0; }else{ \
					/*失败计数*/              pch->readerr++; \
          /*失败几次，就不正常*/    if(pch->readerr>BAO_READ_ERROR_RETYR_TIMES) \
			    /*数据清0*/								{ pch->readok=0;channel_data_clear(i+1);	}} \
					                      }while(0);if(error!=2)t=0

///*===================================================
//                私有函数
//====================================================*/
/*保存数据*///[0] 版本号 [1] 电量 [2] 温度 [3] 故障码 [4-5] 循环次数 [6-7] 容量 [8-9] 电芯电压 [10-11] 电流 (低位在前)
static void save_data(Channel*ch,U8*data)
{
	if(ch==NULL||data==NULL)return;
	ch->Ver 							= data[0];
	ch->Ufsoc							=	data[1];
	ch->Temperature				=	data[2];
	ch->CycleCount				= (((U16)data[5])<<8)|(data[4]);
	ch->RemainingCapacity	= (((U16)data[7])<<8)|(data[6]);
	ch->Voltage          	= (((U16)data[9])<<8)|(data[8]);
	ch->AverageCurrent    = (((U16)data[11])<<8)|(data[10]);
}

/*===================================================
						仓道任务: 读数据
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	static U8 i = 0;
	static Channel *pch;
	static READ_TYPE_MODE mode;
	static U8 buffer[13];
  static U8 error;
	static int timeoutcounter;
	static time_t t = 0;
	PROCESS_BEGIN();
  channel_data_init();//初始化仓道数据
	while(1)
	{
		/*--------------------------------------------------------
		1.对所有通道检查，看充电宝是否有效
		2.有效的充电宝执行以下操作:
		  (1) 读ID 
		  (2) 读数据
		  (3) 读输出标志
		  (4) 6代宝以上,非租借条件下，应该 加密
		---------------------------------------------------------*/
		t=time(0);
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			pch = channel_data_get(i);
			if(pch==NULL)continue;
			if(isvalid_baibi())															                         //判断充电宝是否有效
			{		
				channel_read_start_wait_end(RC_READ_ID,memcpy(pch->id,buffer,10),1000);//读取ID	
				channel_read_start_wait_end(RC_READ_DATA, save_data(pch,buffer),1000); //读取数据

				if(is_ver_6() ||  is_ver_7())                                          //非租借条件下(6代以上，禁止输出,7代可以使用红外通讯禁止输出)
				{
					if((channel_id_is_not_null(pch->id)) && (pch->bao_output==0 || pch->bao_output==BAO_ALLOW || pch->bao_output==BAO_ALLOW_ONE_HOUR))
					{
						channel_read_start_wait_end(RC_LOCK, pch->bao_output=buffer[0],1000);	//加密充电宝
					}
				}
			}
			else//充电宝不存在
			{
				if( (!isvalid_baibi())&&(!isvalid_daowe()) )
					channel_data_clear(i);//数据清0
			}
		}
		//读失败，不延时    读成功：最小延时2.8秒
		if(t!=0) { t= (BAO_READ_DATA_MAX_MS -(time(0)>t?time(0)-t:(0xFFFFFFFF-t+time(0))))%BAO_READ_DATA_MAX_MS;}
		if(t>0){os_delay(channel,t);}
		
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
	}
	PROCESS_END();
}