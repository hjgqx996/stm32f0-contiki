#include "contiki.h"
#include "types.h"


/*===================================================
                仓道任务
====================================================*/

static struct etimer et_channel;
PROCESS(channel_thread, "通道任务");
PROCESS_THREAD(channel_thread, ev, data)  
{
	static U32 timeout = 0;
	BOOL flag=FALSE;
	PROCESS_BEGIN();
	while(1)
	{
		U8 i = 0;
		
		//计时2.8秒
		if(timeout==0)timeout=time(0)+2800;
		if(!((time(0)-timeout)>0x80000000)){
			flag=TRUE;
			timeout=time(0)+2800;
		}
		
		for(;i<5;i++){
			//刷新仓道 状态，告警，异常
			channel_state_check(i);
			channel_warn_check(i);
			channel_error_check(i);
	
			//如果仓道有宝，读取仓道数据.频率2.8秒
			if(flag)
			{}
		}
		flag=FALSE;
		os_delay(et_channel,10);
	}

	PROCESS_END();
}



//AUTOSTART_PROCESSES(channel_thread);

/*===================================================
                iic-仓道任务
====================================================*/
U32 keys[10];
static struct etimer et_iic;
PROCESS(thread_iic, "iic任务");
PROCESS_THREAD(thread_iic, ev, data)  
{
	U8 ids[10];
	PROCESS_BEGIN();
	while(1)
	{
		
		//等待唤醒读取iic消息
		//ld_bq27541_read_id(21,22,ids);
		//ld_bq27541_check_ack(22,21);
		if( ld_key_read(keys) !=0)
		{
			
		}
		os_delay(et_iic,10);
		{
		  static int i = 0;
			i++;
			if(i>200){
				i=0;
				memset(keys,0,sizeof(keys));
			}
		
		}
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(thread_iic);

/*===================================================
                红外-仓道任务
====================================================*/
PROCESS(thread_ir, "ir任务");
PROCESS_THREAD(thread_ir, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		//等待唤醒读取ir
	}

	PROCESS_END();
}
//AUTOSTART_PROCESSES(thread_ir);



