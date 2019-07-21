
/*===================================================
功能:与仓道操作相关函数
====================================================*/
#include "includes.h"
#include "stm32f0xx.h"

/*===================================================
                宏，类型
====================================================*/
/*仓道充电宝编号是否为NULL*/
const U8 null_id[10] = {0,0,0,0,0,0,0,0,0,0};
BOOL channel_id_is_not_null(U8*id){return !buffer_cmp((U8*)null_id,id,CHANNEL_ID_MAX);}

/*所有的仓道数据缓存*/
static Channel chs[CHANNEL_MAX]={0};

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
		ld_iic_init(i+1,channel_config_map[i].io_sda,channel_config_map[i].io_scl);
	}	
}

/*---------------- 清除地址为ch_addr的数据---------- */
BOOL channel_data_clear_by_addr(U8 ch_addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==ch_addr)
		{
			memset((void*)&(chs[i].Ufsoc),0,sizeof(Channel)-((int)&(chs[i].Ufsoc) - (int)&chs[i]));//除地址外，其它清0
			return TRUE;
		}
	}
	return FALSE;
}

/*--------------- 清除第ch(1-n)个仓道的数据----------- */
BOOL channel_data_clear(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	if(pch==NULL)return FALSE;
	memset((void*)&(pch->Ufsoc),0,sizeof(Channel)-((int)&(pch->Ufsoc) - (int)pch));//除地址外，其它清0
	return TRUE;
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
	

/*----------------------------------
仓道运行状态
仓道告警
仓道错误
-----------------------------------*/
void channel_state_check(U8 ch)
{
  Channel*pch = channel_data_get(ch);if(pch==NULL)return;
	
	/*有宝,读取正常*/
	if(isvalid_daowe() && isvalid_baibi() && pch->readok>=2 && channel_id_is_not_null(pch->id))
	{
		pch->state.read_ok=1;
		pch->state.read_error=0;
	}
	
	/*有宝,读取不正常*/
	if(isvalid_daowe() && (pch->readok<2 || channel_id_is_not_null(pch->id)==FALSE))
	{
		pch->state.read_error=1;
		pch->state.read_ok=0;
	}
	/*正在充电*/
	pch->state.charging = isin5v();
	/*-----充电完成--:在充电流程里做--*/
	
	/*-----iic or ir------------------*/
	pch->state.read_from_ir = pch->iic_ir_mode;
}

void channel_warn_check(U8 ch)
{
    #define out5v()  ld_gpio_get(pch->map->io_sw)
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	/*温度*/
	if(pch->Temperature>BAO_WARN_TEMPERATURE)pch->warn.temperature=1;
	else pch->warn.temperature=0;
	
	/*弹仓:在事件中做*/
	
	/*充电告警*/
	if(isin5v() != isout5v())pch->warn.mp=1;
	else pch->warn.mp=0;
}

void channel_error_check(U8 ch)
{
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	//到位开关
	if( (isvalid_daowe()==0) && (pch->state.read_ok==1) )pch->error.daowei=1;
	else pch->error.daowei=0;
	
	//摆臂开关故障
	if(  			( (isvalid_baibi()==0)&&(isvalid_daowe()==1)) 
			|| 		( (isvalid_baibi()==1)&&(isvalid_daowe()==0)&&(pch->state.read_ok==0) )
	  )
	{
		pch->error.baibi=1;
	}else pch->error.baibi=0;
	
	//温度<0
	if( (pch->Temperature<0) || (pch->Temperature>BAO_ERROR_TEMPERATURE) )pch->error.temp=1;
	else pch->error.temp=0;
	
}
//锁仓事件
void channel_lock_event(U8 ch)
{
   

}



/*----------------------------------
仓道申请充电
仓道申请断电
挂起所有充电ms
-----------------------------------*/
void channel_charge(U8 ch)
{


}
void channel_discharge(U8 ch)
{
  //仓道断电 
	
	//从排队的列表中删除本仓道
//	queue_delete(ch);
}
void channel_discharge_all(int ms)
{

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
			if(ch->Ufsoc>CHANNEL_LED_LIGHT_UFSOC && ch->state.read_ok)//电量大于50%灯亮
				ld_gpio_set(ch->map->io_led,HIGH);
			else 
				ld_gpio_set(ch->map->io_led,LOW);
		}
	}
}
