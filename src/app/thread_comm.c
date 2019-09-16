#include "includes.h"
#include "packet.h"

HPacket hpacket;//发送数据包缓冲
/*===================================================
                本地函数
====================================================*/
/*电磁阀动作时，读摆臂开关有效时长*/
static int bai_bi_counter(Channel*pch,int timeout)
{
	int bc = 0;
	if(pch==NULL)return 0;
	while(timeout>0)
	{
		delayms(10);
		if(isvalid_baibi())
			bc++;
		timeout-=10;
	}
	return bc*10;
}
/*一次电磁阀动作 + 灯闪 + 检测成功 
* pch           :仓道数据
* flash_time    :仓道灯闪烁时长
* timeout       :电磁阀动作时长
* hightime_min  :电磁阀动作时长
* check_time    :检测时间
* hightime_max  :摆臂开关最大有效时长 ms
* return        :TRUE or FALSE
*/
BOOL diancifa(Channel*pch,int flashtime,int timeout,int check_time,int hightime_max)
{
	int bcounter=0;
	request_charge_hangup_all(0);//关闭输出,2s
	dian_ci_fa_power(1);         //使能电磁阀电源
	delayms(10);                 //由于595在定时器中被刷新，有一个延时，必须等待，否则重启
	dian_ci_fa(pch,HIGH);        //电磁阀动作
	delayms(500);
	dian_ci_fa(pch,LOW);         //关闭电磁阀
	bcounter = bai_bi_counter(pch,50);//电磁阀打开的时候，读取摆臂开关电平
	dian_ci_fa_power(0);         //关闭电磁阀电源
	if(bcounter < hightime_max){
		channel_led_flash(channel_data_get_index(pch),flashtime);//闪灯	
		return TRUE;
	}
	else return FALSE;
}

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
static void send_lease_state(HPacket*hp, U8 s,U8 ch_addr,U8*ids)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	if(ch==NULL){enable_485_rx();return;}
	data[0]=0x01;		//子命令
	data[1]=s;			//处理结果
	data[2]=ch_addr;//充电宝位置
	memcpy(data+3,ids,CHANNEL_ID_MAX);
	
	//判断租借成功与失败==>标志位
	if(s != Lease_success)
	{
		Channel*pch = channel_data_get_by_addr(ch_addr);
		if(pch!=NULL)
		{
			pch->error.lease=1;//租借失败位，置1
		}
	}
	else
  {
		Channel*pch = channel_data_get_by_addr(ch_addr);
		if(pch!=NULL)
		{
			pch->error.lease=0;//租借失败位，清0
		}	
	}
	
	//发送数据包
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data,system.addr485);
}
//返回归还状态
static void send_return_state(HPacket*hp,U8 s,U8 ch_addr)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	if(ch==NULL){enable_485_rx();return;}
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
	packet_send(hp,0x06,2,send_buf,0xFF);//处理结果	
}

/*===================================================
                具体命令处理函数
====================================================*/
/*发送心跳*/
static void com_send_tick(HPacket*hp)
{
	packet*p = &hp->p;
  U8*data = p->data;
	
	/*长度*/
	int en_len = p->llen;
	if(en_len<=2)en_len=0;
	if(en_len>(2+CHANNEL_MAX))en_len=CHANNEL_MAX;
	
	/*保存充电使能方式*/
	if(p->llen>0)system.enable = (BOOL)data[0]; //背板是否允许充电
	if(p->llen>1)system.mode   = data[1];       //强制 or 自由
	if(en_len)memcpy(system.chs,data+2,en_len); //强制标志 
	#ifndef NOT_USING_IR
	if(p->llen > (2+CHANNEL_MAX))system.iic_ir_mode = (SYSTEM_IIC_IR_MODE)data[2+CHANNEL_MAX];//红外,iic模式选择 :1只iic  2:只ir  3:红外iic
	#endif
	
	/*返回心跳包*/
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
static volatile U8 lch=0;                      							 //仓道位置
static volatile U32 ltimeout=0;                							 //超时时间	
BOOL is_system_lease(void) {return (BOOL)( (lch!=0) && (ltimeout!=0));} //是否租借
AUTOSTART_THREAD_WITHOUT_TIMEOUT(comm_lease)
{
	static HPacket*hp;
	static Channel*pch;
	static U8 buffer[20];
//	static int bcounter = 0;  //电磁阀动作时500,高电平计数
	static int i = 0;
	
	PROCESS_BEGIN();
	while(1){
	  PROCESS_WAIT_EVENT();//等待事件过来
		if(ev== PROCESS_EVENT_COMM_LEASE && data !=NULL)
		{
			hp = (HPacket*)data;                  
			pch = channel_data_get_by_addr(hp->p.data[0]);
			memcpy(buffer,hp->p.data,20);
			send_ack(hp,0x02,0x01);//发送应答
			system.state = SYSTEM_STATE_LEASE;//系统状态:租借
			lch = buffer[0];                  //保存数据:租借位置
			ltimeout = 1000*buffer[11];      //保存数据:租借时间(ms)

			//充电宝编号不对，充电宝编号为0==>differ
			if( (pch==NULL) ||  (buffer_cmp(pch->id,buffer+1,CHANNEL_ID_MAX)==FALSE) || (channel_id_is_not_null(pch->id)==FALSE) ){
				
				#ifdef USING_DEBUG_INFO
				/*pch==NULL:7  !=:6  ==NULL:5 */
				  if(pch==NULL) ld_debug_printf(7,lch,0,0);
				  if(buffer_cmp(pch->id,buffer+1,CHANNEL_ID_MAX)==FALSE) ld_debug_printf(6,lch,0,pch->iic_ir_mode);
				  if(channel_id_is_not_null(pch->id)==FALSE) ld_debug_printf(5,lch,0,pch->iic_ir_mode);
				#endif
				
				send_lease_state(hp,Lease_differ,lch,((pch==NULL)?(buffer+1):(pch->id)));
				goto LEASE_RESET_CONTINUE;
			}//充电宝编号不对
			
			//if(pch->state.read_ok)//充电宝是否有效
			{
				if(!is_ver_5())//非5代包，解锁
				{
					int result=0;
					U8 lock[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
					//尝试解锁几次，当红外忙时，最长等待时间2.5秒
					for(i=0;i<UNLOCK_RETRY_TIMES;i++)
					{
						result = channel_read(pch,RC_UNLOCK,lock,650,(((i+1)==UNLOCK_RETRY_TIMES)?TRUE:FALSE)); //最后一次可以转通讯方式
						if( (result!=TRUE) || (lock[0] != 0x05))
							continue;
						else 
							break;
					}
					if( (result!=TRUE) || (lock[0] != 0x05) ){//解锁失败
						send_lease_state(hp,Lease_decrypt_fall,lch,buffer+1);
						goto LEASE_RESET_CONTINUE;
					}
				}	

				if(diancifa(pch,ltimeout/1000,500,60,20)==FALSE)//电磁阀动作时间500ms,灯闪时间 ltimout/1000秒,摆臂开关(高电平时间<250ms)
				{
					if(channel_check_from_iic(pch))
					{
						send_lease_state(hp,Lease_dianchifa_fall,lch,buffer+1);//电磁阀失败
						pch->error.motor = 1;//电磁阀故障			
					  goto LEASE_RESET_CONTINUE;
					}
				}
				//电磁阀成功
				{
					request_charge_off(channel_data_get_index(pch)); delayms(10);//如果在充电，马上关电
					pch->error.motor = 0;//电磁阀故障清0
					send_lease_state(hp,Lease_success,lch,buffer+1); //成功===>应答包
				  channel_data_clear_by_addr(lch);                 //成功===>清数据
				}
			}
			
			LEASE_RESET_CONTINUE:
			lch=ltimeout=0;
		}
	}
	PROCESS_END();
}

/*归还命令*/
static volatile U8 rch=0;                      							//仓道位置
static volatile U32 rtimeout=0;                							//超时时间	
BOOL is_system_in_return(U8 addr){return (BOOL)((rch!=0)&& (rtimeout!=0) && (rch==addr));}
static void com_return(HPacket*hp)
{
	packet *p = &hp->p;
	Channel*ch = channel_data_get_by_addr(p->data[0]);//仓道地址
  //保存数据
	system.state = SYSTEM_STATE_RETURN;//系统状态:归还//触发归还的异步处理
	rch = p->data[0];                   //保存数据:归还位置
	rtimeout = 1000*p->data[1]+time(0); //保存数据:归还超时(ms)
	channel_led_flash(channel_data_get_index(ch),p->data[1]);//闪灯
	send_ack(hp,0x03,0x01);         //发送应答
}

/*控制命令*/
AUTOSTART_THREAD_WITHOUT_TIMEOUT(comm_ctrl)
{
	static U8 dataout[16];
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
				
			cmd=hp->p.data[0];
			memcpy(bao_id,&hp->p.data[2],10);//充电宝编号
			ctrl_time=hp->p.data[12];        //控制时间
			if(hp->p.llen<13)continue;
			send_ack(hp,0x04,0x01);          //发送应答
			if(cmd==0x00)//设备重启
			{
				send_ctrl_state(hp,cmd,Cmd_success);
				delayms(1000);
				cpu_nvic_reset();
				continue;
			}
			//cmd =01 02 对仓道操作，读取仓道数据
			pch = channel_data_get_by_addr(ch_addr);if(pch==NULL)continue;	
			if(cmd==0x01)//运维打开仓门，使能充电1小时
			{
							
				if(!(data_is_ver_5(bao_id[6])))//6代宝以上，解锁1小时
				{
					 channel_read(pch,RC_UNLOCK_1HOUR,dataout,650,TRUE);
				}
			}
			
			if(cmd==0x01||cmd==0x02)//强制开仓	
			{			

				if(diancifa(pch,ctrl_time,500,60,20))//电磁阀动作时间500ms,灯闪时间ctrl_time秒,摆臂开关(高电平时间<250ms)
				{
					request_charge_off(channel_data_get_index(pch)); //如果在充电，马上关电
					pch->error.motor = 0;                            //电磁阀故障清0
					send_ctrl_state(hp,cmd,Cmd_success );            //成功===>返回应答
					 channel_data_clear_by_addr(ch_addr);            //成功===>清数据
				}
				else
				{
					pch->error.motor = 1;            //电磁阀故障
					send_ctrl_state(hp,cmd,Cmd_fall);//电磁阀失败
				}
			}
		}
    memset(hp,0,sizeof(HPacket));		
		continue;
	 }
		
	PROCESS_END()
}
	
/*进入固件更新*/
AUTOSTART_THREAD_WITHOUT_TIMEOUT(comm_entry)
{
	static HPacket*hp;
	PROCESS_BEGIN();
	while(1)
	{
		PROCESS_WAIT_EVENT();
		if(ev== PROCESS_EVENT_COMM_ENTRY && data !=NULL)
		{
			hp = (HPacket*)data;
			//保存标志 
			system.firmware_updata_flag=0x88;
			ld_flash_write(0,&system.firmware_updata_flag,1,FIRMWARE_updata);
			send_update_state(hp,0x01);
			delayms(1000);
			cpu_nvic_reset();//复位		
		}
	}
	PROCESS_END();
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

/*扩展信息:调试命令*/
#ifdef USING_DEBUG_INFO
static void com_debug_info(HPacket*hp)
{
	/*data[0] = 01 读调试信息   [1] offset [2]counter
	            02信息清0*/
	/*返回 03 读+信息           [04] 信息清成功 0 or 1*/
	if(hp->p.data[0]==01)
	{
		int l = ld_debug_counter();
		int o=hp->p.data[1];
		int c=hp->p.data[2];
		while(o<l&&c>0)
		{
			hp->p.data[0] = 3;	
			ld_debug_read(o,(char*)&hp->p.data[1]);
			packet_send(hp,PC_DEBUGINFO,1+sizeof(DebugInfo),hp->p.data,system.addr485);	
			delayms(20);
			o++;
			c--;			
		}
	}
	else if(hp->p.data[0]==02)
	{
		ld_debug_printf_clear();
		hp->p.data[0] = 4;
		hp->p.data[1] = 1;
		packet_send(hp,PC_DEBUGINFO,2,hp->p.data,system.addr485);
	}
}
#endif

/*命令分配处理函数*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	//地址过滤
	if(p->addr==system.addr485)
		switch(p->cmd)
		{
			case PC_HEART_BREAK	:   ld_system_flash_led(500,5);	com_send_tick(hp);		return;                   //系统灯500ms闪，闪5秒,发送心跳
			case PC_LEASE				:		process_post(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);return; //事件发送给了线程thread_lease
			case PC_RETURN			:		com_return(hp);				return;                                               //记录一下，然后在 return 线程中完成归还动作
			case PC_CTRL				:		process_post(&thread_comm_ctrl,PROCESS_EVENT_COMM_CTRL,(void*)hp);return;   //同步处理,事件发送给了线程thread_lease
			case PC_UPDATE_ENTRY:		process_post(&thread_comm_entry,PROCESS_EVENT_COMM_ENTRY,(void*)hp);return;   //同步处理,事件发送给了线程thread_entry                                               //保存升级标志，复位==>bootloader完成升级
			case PC_UPDATE_MODE :		com_update_mode_query(hp);return;
			                                                
			case PC_UPDATE_DATA://数据放在bootloader接收这里不接收
				
			#ifdef USING_DEBUG_INFO
			case PC_DEBUGINFO   :   com_debug_info(hp);return;  //调试命令 
			#endif
			
			default:break;

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
			return;
			case 0x016: send_handshake_state(hp,0x01); return;//握手
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
	ld_uart_open(COM_485,115200,8,0,1,0,400);  //打开串口
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
                异步归还任务
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(return)
{
	U8 dataout[16];
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
						static int result = 0;
						static int i=0;
						
						//异步归还时，读两次
						for(i=0;i<RETURN_READ_TIMES;i++)
						{
																result = channel_read(pch,RC_READ_ID,dataout,550,TRUE);
							if(result==FALSE) result = channel_read(pch,RC_READ_ID,dataout,550,TRUE);    
							if(result==TRUE)  result = channel_read(pch,RC_READ_DATA,dataout,650,TRUE);
							if(result==TRUE){pch->state.read_ok=1;break;}//成功读取，第一时间标志
						}

						if(!channel_id_is_not_null(pch->id) || (result!=TRUE) )//无法识别
							send_return_state(&hpacket,Return_unrecognized,rch);
						else
							send_return_state(&hpacket,Return_success,rch);
					
						pch->flash=FALSE;//关闪烁
						system.state=SYSTEM_STATE_INIT;//复位
						rch=0;
						rtimeout=0;
					}
				}
			}
		}else{rch=rtimeout=0;}
		os_delay(return,100);
		ld_iwdg_reload();
	}
	PROCESS_END();
}


