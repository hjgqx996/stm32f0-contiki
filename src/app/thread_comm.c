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
	data[0]=0x01;		//子命令
	data[1]=s;			//处理结果
	data[2]=ch_addr;//充电宝位置
	memcpy(data+3,ch->id,CHANNEL_ID_MAX);
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data);
}
//返回归还状态
static void send_return_state(HPacket*hp,U8 s,U8 ch_addr)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	data[0]=0x01;   //子命令
	data[1]=ch_addr;//充电宝位置
	data[2]=s;      //处理结果
  data[3]=ch_addr;//仓道序号	
	data[4]=*((U8*)(&ch->state)); //仓道状态
	data[5]=*((U8*)(&ch->warn));  //仓道告警
	data[6]=*((U8*)(&ch->error)); //仓道错误
	memcpy(data+7,ch->id,10);     //编号
  data[17]=ch->Ver;             //充电宝版本
	data[18]=(U8)(ch->AverageCurrent>>8);//存储电流高8位
	data[19]=(U8)(ch->AverageCurrent&0x00ff);
	data[20]=ch->Ufsoc;//电量
	data[21]=ch->Temperature;//温度
	data[22]=(U8)(ch->CycleCount>>8);//存储循环次数高8位
	data[23]=(U8)(ch->CycleCount&0x00ff);	 
	data[24]=(U8)(ch->RemainingCapacity>>8);//存储电芯容量高8位
	data[25]=(U8)(ch->RemainingCapacity&0x00ff);	 
	data[26]=(U8)(ch->Voltage>>8);//存储电芯电压高8位
	data[27]=(U8)(ch->Voltage&0x00ff);	 
	data[28]=ch->bao_output;//充电宝标志，05允许，06禁止	
	packet_send(hp,0x03,29,data);
}

//回复控制命状态
void send_ctrl_state(HPacket*hp,U8 ctrl,U8 s)
{
	U8 len;
	U8*data=hp->p.data;
	data[0]=0x01;//处理结果
	data[1]=ctrl;//控制命令
	data[2]=s;//处理结果
  packet_send(hp,0x04,3,data);
}

void send_update_state(HPacket*hp,U8 s)
{
	U8 len;
	U8*data=hp->p.data;
	data[0]=s;//处理结果
  packet_send(hp,0x05,1,data);
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
static volatile U8 lch=0;                      							//仓道位置
static volatile U32 ltimeout=0;                							//超时时间	
AUTOSTART_THREAD_WITH_TIMEOUT(comm_lease)
{
	static HPacket*hp;
	static packet* p ;
	static Channel*ch;
	static int lease_unlock_timeout = 0;//解锁超时
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
			
			system.state = LEASE_STATE_LEASE;//系统状态:租借
			lch = p->data[0];      //保存数据:租借位置
			ltimeout = p->data[11];//保存数据:租借时间
		
			if(buffer_cmp(ch->id,p->data+1,CHANNEL_ID_MAX)==FALSE){send_lease_state(hp,Lease_differ,lch);continue;}//充电宝编号不对
			
			if(ch->state.read_ok)//充电宝是否有效
			{
//				is_ver_bt_6(ch->id)//6代宝解锁
//				{
					//解锁一次
//					lease_unlock_timeout=0;
//					while(channel_read_busy(channel_data_get_index(ch),ch->iic_ir_mode))
//					{
//						os_delay(lease,10);
//						lease_unlock_timeout++;
//						if(lease_unlock_timeout>50)goto LEASE_UNLOCK_ERROR;
//					}
//					if(!channel_read_start(channel_data_get_index(ch),ch->iic_ir_mode,ch->iic_dir,RC_UNLOCK))
//						goto LEASE_UNLOCK_ERROR;
//					
//					lease_unlock_timeout=0;
//          while(channel_read_busy(channel_data_get_index(ch),ch->iic_ir_mode))
//					{
//						os_delay(lease,10);
//						lease_unlock_timeout++;
//						if(lease_unlock_timeout>50)goto LEASE_UNLOCK_ERROR;
//					} 
//				  {
//						U8 buf[2];
//						if(channel_read_end(channel_data_get_index(ch),ch->iic_ir_mode,buf)!=2)goto LEASE_UNLOCK_ERROR;
//						
//						//ir-->output==buf[0]
//						if(ch->iic_ir_mode==RTM_IR && ((buf[0]==0x05)||buf[1]==0x07))
//							goto LEASE_NEXT;
//						else
//							goto LEASE_UNLOCK_ERROR;
//						
//						//iic->output应该读一次
//						
//					} 
//					//解锁不成功
//					LEASE_UNLOCK_ERROR:
//						send_lease_state(hp,Lease_decrypt_fall,lch);continue;
//				}
				LEASE_NEXT:
				channel_led_flash(channel_data_get_index(ch),ltimeout);//租借等待时间闪灯	
				channel_discharge_all(500);     //关闭输出
				dian_ci_fa(ch,HIGH);
				os_delay(comm_lease,500);
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
static volatile U8 rch=0;                      							//仓道位置
static volatile U32 rtimeout=0;                							//超时时间	
static void com_return(HPacket*hp)
{
	packet *p = &hp->p;
	Channel*ch = channel_data_get_by_addr(p->data[0]);
	
  //保存数据
	system.state = LEASE_STATE_RETURN;//系统状态:归还
	rch = p->data[0];      //保存数据:租借位置
	rtimeout = p->data[11];//保存数据:租借时间
	
	channel_led_flash(rch,rtimeout);//闪灯
	send_ack(hp,0x03,0x01);         //发送应答
	//触发归还的异步处理
}
/*归还异步处理*/
static void com_return_async(void)
{
  //查询是否归还超时
	
	//发送归还结果数据包
  
}

/*控制命令*/
AUTOSTART_THREAD_WITH_TIMEOUT(comm_ctrl)
{
	static HPacket*hp;
	static packet* p ;
	static Channel*ch;
	static U8 cmd=0;
	static U8 bao_id[10];
	static U8 ctrl_time;
	PROCESS_BEGIN()
	while(1)
	{
		PROCESS_WAIT_EVENT();
		if(ev== PROCESS_EVENT_COMM_CTRL && data !=NULL)
		{
			hp = (HPacket*)data;
			p = &hp->p;
			ch = channel_data_get_by_addr(p->data[1]);	
			cmd=p->data[0];
			memcpy(bao_id,&p->data[2],10);  //充电宝编号
			ctrl_time=p->data[12];
			send_ack(hp,0x04,0x01);         //发送应答
			
			switch(cmd)
			{
				case 00://设备重启
				{
					send_ctrl_state(hp,p->data[0],Cmd_success);
					delayms(1000);
					cpu_nvic_reset();
				}
				break;
				
				case 0x01://运维打开仓门，使能充电1小时
				{
					//6代宝以上，解锁1小时
					if((bao_id[6]&0x0F)>0x06)
					{
					
					}
					//闪灯
					channel_led_flash(channel_data_get_index(ch),ctrl_time);//租借等待时间闪灯	
					channel_discharge_all(500);     //关闭输出
					dian_ci_fa(ch,HIGH);
					os_delay(comm_ctrl,500);
					dian_ci_fa(ch,LOW);//启动电磁阀500ms
					if(KEY_BAI_BI(ch)==0)//摆臂开关
					{
						send_ctrl_state(hp,cmd,Cmd_success );channel_data_clear_by_addr(lch);//成功
					}
					else
						send_ctrl_state(hp,cmd,Cmd_fall);//电磁阀失败
				}
				
				case 0x02://强制开仓
				{
					//闪灯
					
					//电磁阀
					
					//返回状态
				}
				default:
					break;
			}
		}
			continue;
	 }
		
	PROCESS_END()
}
	
/*进入固件更新*/
static void com_update_entry(HPacket*hp)
{
  //保存标志 
	
	send_update_state(hp,0x01);
	cpu_ms_delay(500);
	cpu_nvic_reset();
}

/*更新模式查询*/
static void com_update_mode(HPacket*hp)
{
  U8*data = hp->p.data;
	system.firmware_size=data[0];
	system.firmware_size<<=8;
	system.firmware_size|=data[1];
	system.firmware_size<<=8;
	system.firmware_size|=data[2];
	system.firmware_size<<=8;
	system.firmware_size|=data[3];												 
	system.firmware_version=data[4];
	system.firmware_version<<=8;
	system.firmware_version|=data[5];
	memcpy((void*)system.firmware_MD5,data+6,16);
	send_update_state(hp,normal_model);
}


/*命令分配处理函数*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	//地址过滤
	//if(p->addr!=system.addr485)
	
	switch(p->cmd)
	{
		case PC_HEART_BREAK	:		com_send_tick(hp);		break;
		case PC_LEASE				:		process_post_synch(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);break;  //同步处理,事件发送给了线程thread_lease
		case PC_RETURN			:		com_return(hp);				break;
		case PC_CTRL				:		process_post_synch(&thread_comm_ctrl,PROCESS_EVENT_COMM_CTRL,(void*)hp);break;  //同步处理,事件发送给了线程thread_lease
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
                数据包处理任务
====================================================*/
AUTOSTART_THREAD_WITHOUT_TIMEOUT(packet)
{
	PROCESS_BEGIN();
  memset(&hpacket,0,sizeof(hpacket));         //缓冲清0 	                          
	ld_uart_open(COM_485,115200,8,0,1,0,1050);  //打开串口
	enable_485_rx();                            //使能接收
	while(1)
	{	
		PROCESS_WAIT_EVENT();                           //等待中断接收发来的事件(成功接收一个数据包事件)
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)//接收到一个有效的数据包
		{		
		  com_process(&hpacket);												//处理数据包
			memset(&hpacket,0,sizeof(hpacket));           //缓冲清0
		}
	}
	PROCESS_END();
}


AUTOSTART_THREAD_WITH_TIMEOUT(comm)
{
	PROCESS_BEGIN();
	os_delay(comm,500);
	while(1)
	{	
		com_return_async();   //归还异步处理
		os_delay(comm,80);
	}
	PROCESS_END();
}


