#include "includes.h"
#include "packet.h"


HPacket hpacket;

/*===================================================
                本地函数
====================================================*/
//应答
static void send_ack(HPacket*hp, U8 cmd,U8 result)
{
	U8 send_buf[2];
	send_buf[0]=0x00;//控制命令
	send_buf[1]=result;//处理结果
	packet_send(hp,cmd,2,send_buf);
}
/*===================================================
                具体命令处理函数
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
			data[offset++]=ch->Voltage>>8;
			data[offset++]=ch->Voltage&0xff;//电芯电压
			data[offset++]=ch->bao_output; //充电宝输出标志 
		}
		packet_send(hp,PC_HEART_BREAK,offset,data);
	}
}
/*租借命令*/
static void com_lease(HPacket*hp)
{
	//保存数据包数据
	
	//租借等待时间闪灯
	
	//发送应答
	send_ack(hp,0x02,0x01);
	
	//充电宝是否有效
	
	//充电宝编号是否一致
	
	//6代宝解锁
	
	//启动电磁阀，并闪灯
	
	//返回操作结果

}

/*归还命令*/
static void com_return(HPacket*hp)
{
   //保存数据
	
	 //闪灯
	
	//发送应答
	send_ack(hp,0x03,0x01);
	
	//触发归还的异步处理
}
/*归还异步处理*/
static void com_return_async(void)
{
  //查询是否归还超时
	
	//发送归还结果数据包

}




/*控制命令*/
static void com_ctrl(HPacket*hp)
{

}

/*进入固件更新*/
static void com_update_entry(HPacket*hp)
{

}

/*更新模式查询*/
static void com_update_mode(HPacket*hp)
{

}


/*通道处理函数*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	//地址过滤
	//if(p->addr!=system.addr485)
	
	switch(p->cmd)
	{
		case PC_HEART_BREAK	:		com_send_tick(hp);		break;
		case PC_LEASE				:		com_lease(hp);	    	break;
		case PC_RETURN			:		com_return(hp);				break;
		case PC_CTRL				:		com_ctrl(hp);					break;
		case PC_UPDATE_ENTRY:		com_update_entry(hp);	break;
		case PC_UPDATE_MODE :		com_update_mode(hp);	break;
		
		//数据放在bootloader接收这里不接收
		case PC_UPDATE_DATA:
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
		
		//等待事件，超时50ms
		wait_event(et_comm,50);
		
		//接收到一个有效的数据包
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)
		{
			//处理数据包
		  com_process(&hpacket);
			memset(&hpacket,0,sizeof(hpacket));
		}
		
		//异步处理归还命令(接收数据时,发送数据可能有问题)
		com_return_async();
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(thread_comm);