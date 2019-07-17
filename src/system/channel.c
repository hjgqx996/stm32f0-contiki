
/*===================================================
功能:与仓道操作相关函数
====================================================*/
#include "includes.h"

/*===================================================
                配置文件
====================================================*/

/*===================================================
                宏，类型
====================================================*/
/*仓道充电宝编号是否为NULL*/
const U8 null_id[10] = {0,0,0,0,0,0,0,0,0,0};
BOOL channel_id_is_not_null(U8*id){return buffer_cmp((U8*)null_id,id,CHANNEL_ID_MAX);}
static Channel chs[CHANNEL_MAX]={0};
/*===================================================
                本地函数
====================================================*/


/*===================================================
                全局函数
====================================================*/
/*
* channel数据初始化
*/
BOOL channel_data_init(void)
{
	int i =0;
	memset(chs,0,sizeof(chs));
	for(;i<CHANNEL_MAX;i++)
	{
		chs[i].map = &channel_config_map[i]; //io
		//chs[i].addr = //addr 仓道地址
		ld_ir_init(i+1,channel_config_map[i].io_ir,channel_config_map[i].io_re);
	}	
	ld_ir_timer_init();//红外配置
}
BOOL channel_data_clear_by_addr(U8 ch_addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==ch_addr)
		{
			memset(&chs[i],0,sizeof(Channel));
			return TRUE;
		}
	}
	return FALSE;
}


/*获取仓道数据
*channel:1-n
*/
Channel*channel_data_get(U8 channel)
{
	channel-=1;
	if(channel>=CHANNEL_MAX)return NULL;
	return &chs[channel];
}
/*获取仓道数据--by addr
*channel:1-n
*/
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
/*----------------------------------
6代宝操作:加密 解密
-----------------------------------*/



/*----------------------------------
仓道运行状态
仓道告警
仓道错误
-----------------------------------*/
void channel_state_check(U8 ch)
{

}

void channel_warn_check(U8 ch)
{

}

void channel_error_check(U8 ch)
{

}

/*----------------------------------
仓道申请充电
仓道申请断电
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
				if(ch->flash_ms<LEASE_LED_FLASH_TIME){
					ch->flash=FALSE;
					ld_gpio_set(ch->map->io_led,LOW);
				}//停止闪烁
				else
				ld_gpio_set(ch->map->io_led,ld_gpio_get(ch->map->io_led)==0?HIGH:LOW);
			}
		}
		else{
			//电量大于50%灯亮
			if(ch->Ufsoc>CHANNEL_LED_LIGHT_UFSOC && 1/*充电宝有效()*/)
			{
				ld_gpio_set(ch->map->io_led,HIGH);
			}
		}
	}

}