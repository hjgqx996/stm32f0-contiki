#include "includes.h"
#include "packet.h"

HPacket hpacket;


/*解锁一次充电宝*/
#define UnlockBao ReadBao

/*cmd:命令  timeout:超时ms, thread:线程名,fail:失败时运行的代码  sucess:成功时运行的代码, size:返回数据缓存大小*/
#define ReadBao(cmd,timeout,thread,fail,sucess,size)  {\
	/* 重入变量:计时 */  static S16 to = timeout/10;static S8 err=0; static U8 dataout[size]; \
  /* 发送命令      */	 do{err = channel_read(pch,cmd,dataout);to--;os_delay(thread,10); \
	/* 超时退出*/        if(to<0){err=4;break;}}while(err<2); \
	/* 成功*/            if(err==2)sucess \
	/* 失败*/            else fail}



/*===================================================
                本地函数
====================================================*/
//应答
static void send_ack(HPacket*hp, U8 cmd,U8 result)
{
	U8 send_buf[2];
	send_buf[0]=0x00;//控制命令
	send_buf[1]=result;//处理结果
	packet_send(hp,cmd,2,send_buf,system.addr485);
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
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data,system.addr485);
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
	packet_send(hp,0x03,29,data,system.addr485);
}

//回复控制命状态
static void send_ctrl_state(HPacket*hp,U8 ctrl,U8 s)
{
	U8*data=hp->p.data;
	data[0]=0x01;//处理结果
	data[1]=ctrl;//控制命令
	data[2]=s;//处理结果
  packet_send(hp,0x04,3,data,system.addr485);
}
//回复升级状态
static void send_update_state(HPacket*hp,U8 s)
{
	U8*data=hp->p.data;
	data[0]=s;//处理结果
  packet_send(hp,0x05,1,data,system.addr485);
}

//烧录器回复0x16
void send_handshake_state(HPacket*hp,U8 s)
{
	packet_send(hp,0x16,1,&s,0xFF);//处理结果	 
}

//烧录器回复0x06
void send_add_state(HPacket*hp,U8 add,U8 s)
{
	uint8_t send_buf[2];
	send_buf[0]=add;//控制命令
	send_buf[1]=s;//处理结果		
	packet_send(hp,0x06,2,&s,0xFF);//处理结果	
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
	data[4] = SOFTWARE_VERSION>>8;    //软件编号
	data[5] = SOFTWARE_VERSION&0x00FF;//软件编号
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
		packet_send(hp,PC_HEART_BREAK,offset,data,system.addr485);
		memset(hp,0,sizeof(HPacket));
	}
}

/*租借命令*/
static volatile U8 lch=0;                      							//仓道位置
static volatile U32 ltimeout=0;                							//超时时间	
AUTOSTART_THREAD_WITH_TIMEOUT(comm_lease)
{
	static HPacket*hp;
	static Channel*pch;
	PROCESS_BEGIN();
	while(1){
	  PROCESS_WAIT_EVENT();//等待事件过来
		if(ev== PROCESS_EVENT_COMM_LEASE && data !=NULL)
		{
			hp = (HPacket*)data;                  
			pch = channel_data_get_by_addr(hp->p.data[0]);
			
			send_ack(hp,0x02,0x01);//发送应答
			if(pch==NULL)break;
			
			system.state = SYSTEM_STATE_LEASE;//系统状态:租借
			lch = hp->p.data[0];      //保存数据:租借位置
			ltimeout = 1000*hp->p.data[11];//保存数据:租借时间(ms)
		
			if(buffer_cmp(pch->id,hp->p.data+1,CHANNEL_ID_MAX)==FALSE){send_lease_state(hp,Lease_differ,lch);continue;}//充电宝编号不对
			
			if(pch->state.read_ok)//充电宝是否有效
			{
				if(is_ver_6() || is_ver_7())//6代宝解锁,600ms,失败返回应答包
					UnlockBao(RC_UNLOCK,600,comm_lease,{},{send_lease_state(hp,Lease_decrypt_fall,lch);continue;},2);
					
				channel_led_flash(channel_data_get_index(pch),ltimeout);//闪灯	
				request_charge_hangup_all(1);//关闭输出,1s
				dian_ci_fa(pch,HIGH);      //电磁阀动作
				os_delay(comm_lease,500);  //延时500ms
				dian_ci_fa(pch,LOW);       //关闭电磁阀
				if(!isvalid_baibi())       //摆臂开关
				{
					send_lease_state(hp,Lease_success,lch); //成功===>应答包
				  channel_data_clear_by_addr(lch);        //成功===>清数据
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
	system.state = SYSTEM_STATE_RETURN;//系统状态:归还//触发归还的异步处理
	rch = p->data[0];      //保存数据:归还位置
	rtimeout = p->data[11];//保存数据:归还超时
	
	channel_led_flash(rch,rtimeout);//闪灯
	send_ack(hp,0x03,0x01);         //发送应答
}

/*控制命令*/
AUTOSTART_THREAD_WITH_TIMEOUT(comm_ctrl)
{
	static HPacket*hp;
	static Channel*pch;
	static U8 cmd=0;
	static U8 bao_id[10];
	static U8 ctrl_time;
	static U8 ch_addr = 0;
	PROCESS_BEGIN()
	while(1)
	{
		PROCESS_WAIT_EVENT();
		if(ev== PROCESS_EVENT_COMM_CTRL && data !=NULL)
		{
			hp = (HPacket*)data;
			ch_addr = hp->p.data[1];
			pch = channel_data_get_by_addr(ch_addr);if(pch==NULL)continue;		
			cmd=hp->p.data[0];
			memcpy(bao_id,&hp->p.data[2],10);//充电宝编号
			ctrl_time=hp->p.data[12];        //控制时间
			send_ack(hp,0x04,0x01);          //发送应答
			
			if(cmd==0x01)//设备重启
			{
				send_ctrl_state(hp,hp->p.data[0],Cmd_success);
				delayms(1000);
				cpu_nvic_reset();
				continue;
			}
				
			if(cmd==0x02)//运维打开仓门，使能充电1小时
			{
				if((bao_id[6]&0x0F)>0x06)//6代宝以上，解锁1小时
				{
					 UnlockBao(RC_UNLOCK_1HOUR,1000,comm_ctrl,{},{},2);
				}
			}
			
			if(cmd==0x02||cmd==0x03)//强制开仓	
			{					
				channel_led_flash(channel_data_get_index(pch),ctrl_time);//闪灯	
				request_charge_hangup_all(1);//关闭输出,1s
				dian_ci_fa(pch,HIGH);      //电磁阀动作
				os_delay(comm_ctrl,500);   //延时500ms
				dian_ci_fa(pch,LOW);       //关闭电磁阀
				if(!isvalid_baibi())       //摆臂开关
				{
					send_ctrl_state(hp,cmd,Cmd_success );        //成功===>返回应答
					 channel_data_clear_by_addr(ch_addr);        //成功===>清数据
				}
				else
					send_ctrl_state(hp,cmd,Cmd_fall);//电磁阀失败
			}
		}
    memset(hp,0,sizeof(HPacket));		
		continue;
	 }
		
	PROCESS_END()
}
	
/*进入固件更新*/
static void com_update_entry(HPacket*hp)
{
  //保存标志 
	system.firmware_updata_flag=0x88;
	ld_flash_write(0,&system.firmware_updata_flag,1,FIRMWARE_updata);
	send_update_state(hp,0x01);
	cpu_ms_delay(500);
	cpu_nvic_reset();//复位
}

/*更新模式查询*/
static void com_update_mode_query(HPacket*hp)
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
	if(p->addr==system.addr485)
		switch(p->cmd)
		{
			case PC_HEART_BREAK	:   ld_system_flash_led(2000);	com_send_tick(hp);		return;                         //系统灯2000ms闪，发送心跳
			case PC_LEASE				:		process_post(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);return; //同步处理,事件发送给了线程thread_lease
			case PC_RETURN			:		com_return(hp);				return;
			case PC_CTRL				:		process_post(&thread_comm_ctrl,PROCESS_EVENT_COMM_CTRL,(void*)hp);return;   //同步处理,事件发送给了线程thread_lease
			case PC_UPDATE_ENTRY:		com_update_entry(hp);	break;
			case PC_UPDATE_MODE :		com_update_mode_query(hp);break;
			
			case PC_UPDATE_DATA://数据放在bootloader接收这里不接收
			default:break;
			break;
		}	
	//烧录工具通讯
	else if(p->addr==0xFE)
	{
		switch(p->cmd)
		{
			case 0x06://烧写地址
				memcpy((void*)&system.addr485,p->data,1+CHANNEL_MAX);             //保存到系统运行数据中
			  channel_addr_set((U8*)system.addr_ch);                            //保存到仓道运行数据中
				ld_flash_write(0,(U8*)&system.addr485, 1+CHANNEL_MAX, RS485_ADDR);//写入flash保存
				send_add_state(hp,system.addr485,0x01);
			break;
			case 0x016: send_handshake_state(hp,0x01); break;//握手
			default:break;
		}
	}
		memset(hp,0,sizeof(HPacket));
		enable_485_rx();	
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
		  com_process(&hpacket);												//处理数据包
		ld_iwdg_reload();
	}
	PROCESS_END();
}

/*===================================================
                归还异步操作
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(return)
{
	static Channel*pch;
	PROCESS_BEGIN();
	os_delay(return,500);
	
	while(1)
	{	
		if(system.state == SYSTEM_STATE_RETURN)
		{
			if(istimeout(time(0),rtimeout))//超时
			{
				send_return_state(&hpacket,Return_timeout,rch);//发送超时归还包
				system.state=SYSTEM_STATE_INIT;//复位状态
				rch=0;
				rtimeout=0;
			}else{
				pch=channel_data_get_by_addr(rch);
				if(isvalid_baibi())//有归还，识别一下充电宝
				{
				  os_delay(return,20);//20ms去抖
					if(isvalid_baibi())
					{
						static int i=0;
						for(;i<3;i++)
						{
							//尝试3次读取充电宝信息,每次超时3000ms
							if(!channel_id_is_not_null(pch->id))
								ReadBao(RC_READ_ID,3000,return,{},{memcpy(pch->id,dataout,10);break;},10);					
						}	
						if(!channel_id_is_not_null(pch->id))//无法识别
							send_return_state(&hpacket,Return_unrecognized,rch);
						else
							send_return_state(&hpacket,Return_success,rch);
					
						pch->flash=FALSE;//关闪烁
						system.state=SYSTEM_STATE_INIT;//复位
					}
				}
			}
		}
		os_delay(return,80);
		ld_iwdg_reload();
	}
	PROCESS_END();
}


