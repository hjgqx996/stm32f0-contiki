#include "includes.h"
#include "packet.h"


HPacket hpacket;
/*===================================================
                本地函数
====================================================*/

/*发送心跳*/
static void com_send_tick(HPacket*hp)
{
	packet*p = &hp->p;
  U8*data = p->data;
	data[0]=system.addr485;
	data[1]=CHANNEL_MAX;//仓道数量
	data[2]=HARDWARE_VERSION>>8;    //硬件版本
	data[3]=HARDWARE_VERSION&0x00FF;//硬件版本
	
	//软件编号
	data[4] = SOFTWARE_VERSION>>8;
	data[5] = SOFTWARE_VERSION&0x00FF;
	{
		U8 i=0;
		U8 offset=6;
		for(;i<CHANNEL_MAX;i++)
		{
			Channel*ch = channel_data_get(i+1);
			if(ch==NULL)return;
			data[offset++]=ch->addr;//仓道地址
			data[offset++]=*((U8*)(&ch->state)); //仓道状态
			data[offset++]=*((U8*)(&ch->warn));  //仓道告警
			data[offset++]=*((U8*)(&ch->error));  //仓道错误
			memcpy(data+offset,ch->id,CHANNEL_ID_MAX);offset+=CHANNEL_ID_MAX;//仓道编号
			data[offset++]=ch->Ver;//充电宝版本
			data[offset++]=ch->AverageCurrent>>8;
			data[offset++]=ch->AverageCurrent&0xFF;//平均电流
			data[offset++]=ch->Ufsoc;//剩余电量
			data[offset++]=ch->Temperature;//温度
			data[offset++]=ch->CycleCount>>8;
			data[offset++]=ch->CycleCount&0xff;//循环次数
			data[offset++]=ch->RemainingCapacity>>8;
			data[offset++]=ch->RemainingCapacity&0xFF;//剩余容量
			data[offset++]=ch->bao_output; //充电宝输出标志 
		}
		packet_send(hp,PC_HEART_BREAK,offset,data);
	}
}


/*通道处理函数*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	switch(p->cmd)
	{
		case PC_HEART_BREAK:
			com_send_tick(hp);
		break;
	
	  default:
			memset(hp,0,sizeof(HPacket));
		  enable_485_rx();
		break;
	}
}

/*===================================================
                全局函数
====================================================*/
static struct etimer et_comm;
PROCESS(thread_comm, "通讯任务");
PROCESS_THREAD(thread_comm, ev, data)  
{
	PROCESS_BEGIN();
  memset(&hpacket,0,sizeof(hpacket));
	os_delay(et_comm,1000);//延时1秒再通讯       
	ld_uart_open(COM_485,115200,8,0,1,0,1050);
	disable_485_tx();//允许接收
	while(1)
	{
		PROCESS_WAIT_EVENT();
		//接收到一个有效的数据包
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)
		{
			//处理数据包
		  com_process(&hpacket);
			memset(&hpacket,0,sizeof(hpacket));
		}
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(thread_comm);