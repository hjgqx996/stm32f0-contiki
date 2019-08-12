
/*===================================================
功能:与仓道操作相关函数
====================================================*/
#include "includes.h"
#include "stm32f0xx.h"

/*===================================================
                宏，类型
====================================================*/
/*仓道充电宝编号是否为NULL*/
static const U8 null_id[10] = {0,0,0,0,0,0,0,0,0,0};
BOOL channel_id_is_not_null(U8*id){return (buffer_cmp((U8*)null_id,id,CHANNEL_ID_MAX)==TRUE)?FALSE:TRUE;}

/*所有的仓道数据缓存*/
static Channel chs[CHANNEL_MAX]={0};



/*===================================================
                本地函数
====================================================*/
/*-------------------------------------
仓道运行状态
-------------------------------------*/
static void channel_state_check(U8 ch)
{
  Channel*pch = channel_data_get(ch);if(pch==NULL)return;
	
	/*有宝,读取正常*/
	if(isvalid_daowe() && isvalid_baibi() && is_readok() && channel_id_is_not_null(pch->id))
	{
		pch->state.read_ok=1;
		pch->state.read_error=0;
	}else pch->state.read_ok=0;
	
	/*有宝,读取不正常*/
	if( (isvalid_daowe()) && ( (is_readerr()) /*|| (channel_id_is_not_null(pch->id)==FALSE)*/ ) )
	{
		pch->state.read_error=1;
		pch->state.read_ok=0;
	}else pch->state.read_error=0;
	/*正在充电*/
	pch->state.charging = isin5v();
	/*-----充电完成--:在充电流程里做--*/
	
	/*-----iic or ir------------------*/
	if(isvalid_baibi())
		pch->state.read_from_ir = pch->iic_ir_mode;
	else 
		pch->state.read_from_ir = 0;
}
/*-------------------------------------
仓道告警
-------------------------------------*/
static void channel_warn_check(U8 ch)
{
	U8 d = 0;
    #define out5v()  ld_gpio_get(pch->map->io_sw)
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	/*温度告警:<0 or >60*/
	if( (pch->Temperature>BAO_WARN_TEMPERATURE) || (pch->Temperature<0) )pch->warn.temperature=1;
	else pch->warn.temperature=0;
	
	/*弹仓:在事件中做*/
	
	/*5V充电告警：5v输入输出电平不同*/
	d = 0;
	if(isin5v() != isout5v())
	{
		delayus(50);
		if(isin5v() != isout5v())
		{
			delayus(50);		
			if(isin5v() != isout5v())
			{
			 d = pch->warn.mp=1;
			}
		}
	}
  pch->warn.mp = d;			
}
/*-------------------------------------
仓道错误
-------------------------------------*/
static void channel_error_check(U8 ch)
{
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	//到位开关
	if( (isvalid_daowe()==0) && (pch->state.read_ok==1) )pch->error.daowei=1;
	else pch->error.daowei=0;
	
	//摆臂开关故障
	if(  			( (isvalid_baibi()==0)&&(isvalid_daowe()==1))                            //摆臂无，到位有
			|| 		( (isvalid_baibi()==1)&&(isvalid_daowe()==0)&&(pch->state.read_ok==0) )  //摆臂有，到位无，读不到宝
	  )
	{
		pch->error.baibi=1;
	}else pch->error.baibi=0;
	
	//温度 <0 or >65:来电宝故障0x20
	if( (pch->Temperature<0) || (pch->Temperature>BAO_ERROR_TEMPERATURE) )pch->error.temp=1;
	else pch->error.temp=0;
	
	//顶针识别故障,识别计数>=2,并且摆臂开关非故障下
	if( (pch->iic_error_counter>=BAO_DINGZHEN_ERROR_TIMES) && (pch->error.baibi==0) )
		pch->error.thimble=1; 
	else pch->error.thimble = 0;
	
	//红外识别故障
	if( (pch->ir_error_counter>=BAO_IR_ERROR_TIMES)  && (pch->error.baibi==0))
		pch->error.ir=1;
	else 
	{
		pch->error.ir=0;
	}
	
	//电磁阀故障===>状态在电磁阀动作时 作判断，不在此判断
	
	//借宝故障  ===>在租借命令回复包中 作判断，不在此判断
	//借宝指令返回 不成功，被认为是 借宝故障
	
	/*---------------开关故障闪灯-----------------------------------*/
	if(pch->error.baibi || pch->error.daowei) 
		ld_system_flash_led(100,2); //开关错误，100ms,闪2秒
}
/*===================================================
                全局函数
====================================================*/

/*--------------------channel数据初始化-------------*/
BOOL channel_data_init(void)
{
	int i =0;
	memset(chs,0,sizeof(chs));
	for(;i<CHANNEL_MAX;i++)
	{
		chs[i].map = &channel_config_map[i]; //io
		ld_ir_init(i+1,channel_config_map[i].io_ir,channel_config_map[i].io_re);
	}	
	return TRUE;
}


/*--------------- 清除第ch(1-n)个仓道的数据----------- */
BOOL channel_data_clear(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	if(pch==NULL)return FALSE;
	memset((void*)&(pch->Ufsoc),0,sizeof(Channel)-((int)&(pch->Ufsoc) - (int)pch));//除地址外，其它清0
	memset((void*)&(pch->state),0,1);                  //状态位清
	pch->warn.temperature=0;                           //温度报警清
	pch->error.ir=pch->error.temp=pch->error.thimble=0;//错误状态清
	pch->iic_error_counter = pch->ir_error_counter = 0;//顶针，红外错误计数清
	pch->readerr = pch->readok = 0;
	return TRUE;
}

/*---------------- 清除地址为ch_addr的数据---------- */
BOOL channel_data_clear_by_addr(U8 ch_addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==ch_addr)
		{
			memset((void*)&(chs[i].first_insert),0,sizeof(Channel)-((int)&(chs[i].first_insert) - (int)(&chs[i])));//除地址外，其它清0
		}
	}
	return FALSE;
}

/*-------------- 获取仓道数据 channel:1-n----------------*/
Channel*channel_data_get(U8 channel)
{
	channel-=1;
	if(channel>=CHANNEL_MAX)return NULL;
	return &chs[channel];
}
/*--------------- 获取仓道数据--by addr-------------------*/
Channel*channel_data_get_by_addr(U8 addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==addr)
			return &chs[i];
	}
	return NULL;
}
/*------------获取仓道索引,error:return <0: return :1-n-----*/
int channel_data_get_index(Channel*ch)
{
	U32 coffset = 0;
	U8 index = 0;
	coffset = (U32)ch - (U32)chs;
	if(coffset%sizeof(Channel)==0)index=coffset/sizeof(Channel);
	else return -1;
	if(index>=CHANNEL_MAX)return -1;
	return index+1;
}
/*-------------设置仓道地址----------------------------------*/
void channel_addr_set(U8*addrs)
{
	int i=0;Channel *vch;
	for(i=0;i<=CHANNEL_MAX;i++)
	{
		vch=channel_data_get(i+1);if(vch==NULL)continue;
		vch->addr=addrs[i];//保存通道地址
	}
}

/*-------------保存数据----------------------------------*/
//[0] 版本号 [1] 电量 [2] 温度 [3] 故障码 [4-5] 循环次数 [6-7] 容量 [8-9] 电芯电压 [10-11] 电流 (低位在前)
void channel_save_data(Channel*ch,U8*data)
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
/*-------------仓道状态1秒检一次----------------------------------*/
void channel_check_timer_1s(void)
{
	int i=1;
	for(;i<=CHANNEL_MAX;i++)
	{
		channel_state_check(i);
		channel_warn_check(i);
		channel_error_check(i);	
	}		
}

/*----------------------------------
   仓道灯
ch:仓道号   seconds:闪烁时间  timer_ms:定义器时间
租借时，仓道灯闪烁频率0.5s
-----------------------------------*/
void channel_led_flash(U8 ch,U8 seconds)
{
  Channel*channel = channel_data_get(ch);
	if(channel==NULL)return;
	channel->flash=TRUE;
	channel->flash_ms=1000*seconds;
	channel->flash_now=0;
}
void channels_les_flash_timer(int timer_ms)
{
	//查询所有通道,管理仓道灯
	int i=0;
	for(;i<CHANNEL_MAX;i++)
	{
		Channel*ch =channel_data_get(i+1);
		if(ch->map==NULL)continue;
		if(ch->flash){
			ch->flash_now+=timer_ms;
			if(ch->flash_now>=LEASE_LED_FLASH_TIME)
			{	
				//时间递减
				ch->flash_now-=LEASE_LED_FLASH_TIME;
				ch->flash_ms-=LEASE_LED_FLASH_TIME;
				if(ch->flash_ms<LEASE_LED_FLASH_TIME)
				{
					ch->flash=FALSE;
					ld_gpio_set(ch->map->io_led,LOW);//停止闪烁
				}
				else
					ld_gpio_set(ch->map->io_led,ld_gpio_get(ch->map->io_led)==0?HIGH:LOW);
			}
		}
		else{
			if( (ch->Ufsoc>CHANNEL_LED_LIGHT_UFSOC) && (ch->state.read_ok) )//电量大于50%灯亮
				ld_gpio_set(ch->map->io_led,HIGH);
			else 
				ld_gpio_set(ch->map->io_led,LOW);
		}
	}
}
