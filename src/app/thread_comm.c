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

//返回租借状态
static void send_lease_state(HPacket*hp, U8 s,U8 ch_addr)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	data[0]=0x01;//子命令
	data[1]=s;//处理结果
	data[2]=ch_addr;
	memcpy(data+3,ch->id,CHANNEL_ID_MAX);
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data);
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
static volatile LeaseState ls=LEASE_STATE_INIT;              //系统租借状态
static volatile U8 lch=0;                      							//仓道位置
static volatile U32 ltimeout=0;                							//超时时间	

static struct etimer et_comm_lease;
PROCESS(thread_comm_lease, "租借处理 任务");
AUTOSTART_PROCESSES(thread_comm_lease);
PROCESS_THREAD(thread_comm_lease, ev, data)  
{
	static HPacket*hp;
	static packet* p ;
	static Channel*ch;
	PROCESS_BEGIN();
	while(1){
	  PROCESS_WAIT_EVENT();//等待事件过来
		if(ev== PROCESS_EVENT_COMM_LEASE && data !=NULL)
		{
			hp = (HPacket*)data;
			p = &hp->p;
			ch = channel_data_get_by_addr(p->data[0]);
			
			send_ack(hp,0x02,0x01);//发送应答
			if(ch==NULL)break;
			
			ls = LEASE_STATE_LEASE;
			lch = p->data[0];
			ltimeout = p->data[11];//保存数据包数据
		
			if(buffer_cmp(ch->id,p->data+1,CHANNEL_ID_MAX)==FALSE){send_lease_state(hp,Lease_differ,lch);continue;}//充电宝编号不对
			
			if(ch->state.read_ok)//充电宝是否有效
			{
				is_6_battery(ch->id)//6代宝解锁
				{
					//解锁一次
					
					//查询输出标志 -->
					
					send_lease_state(hp,Lease_decrypt_fall,lch);continue;//解锁不成功
				}
				channel_led_flash(lch,ltimeout);//租借等待时间闪灯	
				//关闭输出
				dian_ci_fa(ch,HIGH);
				os_delay(et_comm_lease,500);
				dian_ci_fa(ch,LOW);//启动电磁阀500ms
				if(KEY_BAI_BI(ch)==0)//摆臂开关
				{
				  send_lease_state(hp,Lease_success,lch);channel_data_clear_by_addr(lch);//成功
				}
				else
					send_lease_state(hp,Lease_dianchifa_fall,lch);//电磁阀失败
			}
			continue;
		}
	}
	PROCESS_END();
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
		case PC_LEASE				:		process_post_synch(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);break;  //同步处理事件
		case PC_RETURN			:		com_return(hp);				break;
		case PC_CTRL				:		com_ctrl(hp);					break;
		case PC_UPDATE_ENTRY:		com_update_entry(hp);	break;
		case PC_UPDATE_MODE :		com_update_mode(hp);	break;
	
		case PC_UPDATE_DATA://数据放在bootloader接收这里不接收
	  default:	
			memset(hp,0,sizeof(HPacket));
      enable_485_rx();
		break;
	}			
}

/*===================================================
                全局函数
====================================================*/
PROCESS(thread_packet, "数据包处理任务");
AUTOSTART_PROCESSES(thread_packet);
PROCESS_THREAD(thread_packet, ev, data)  
{
	PROCESS_BEGIN();
  memset(&hpacket,0,sizeof(hpacket));
	ld_uart_init(); 	
	ld_uart_open(COM_485,115200,8,0,1,0,1050);
	enable_485_rx();
	while(1)
	{	
		PROCESS_WAIT_EVENT();
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)//接收到一个有效的数据包
		{		
		  com_process(&hpacket);//处理数据包
			memset(&hpacket,0,sizeof(hpacket));
		}
	}
	PROCESS_END();
}


static struct etimer et_comm;
PROCESS(thread_comm, "通讯任务");
AUTOSTART_PROCESSES(thread_comm);
PROCESS_THREAD(thread_comm, ev, data)  
{
	PROCESS_BEGIN();
	os_delay(et_comm,500);
	while(1)
	{	
		com_return_async();   //归还异步处理
		os_delay(et_comm,80);
	}
	PROCESS_END();
}


