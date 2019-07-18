#include "includes.h"

//到位开关有效
#define isvalid_daowe()  ld_gpio_get(ch->map->io_detect) 
//摆臂开关有效
#define isvalid_baibi()  ld_gpio_get(ch->map->io_sw) 

//错误计数，判断选择哪种方式来通讯
static READ_TYPE_MODE iic_ir_select_poll(Channel*ch,BOOL error)
{
	READ_TYPE_MODE mode;
  if(system.iic_ir_mode==SIIM_ONLY_IR) {return RTM_IR;}	//强制使用 ir		
	if(system.iic_ir_mode==SIIM_ONLY_IIC){mode = RTM_IIC;goto READ_TYPE_MODE_IIC;}//强制使用 iic	
	
	READ_TYPE_MODE_IIC_IR://方式切换
  mode = ch->iic_ir_mode;
	if(mode==RTM_IR)
		ch->iic_ir_mode_counter+=(error==TRUE)?1:0;//切换错误累加
	
	if(ch->iic_ir_mode_counter>=IIC_IR_SWITCH_ERROR_MAX)//切换
	{
		ch->iic_ir_mode=0x01&(!ch->iic_ir_mode);
		ch->iic_dir_counter=ch->iic_ir_mode_counter=0;
		mode = ch->iic_ir_mode;
		return mode;
	}
	
	READ_TYPE_MODE_IIC://方向切换
	ch->iic_dir_counter+=(error==TRUE)?1:0;//错误累加
	if(ch->iic_dir_counter>=IIC_DIR_SWITCH_MAX)
	{
		ch->iic_dir=(!ch->iic_dir)&0x01;
		ch->iic_dir_counter=0;	
		ch->iic_ir_mode_counter++;//切换错误累加
	}
	return mode;
}
/*保存数据*/
static void save_data(Channel*ch,U8*data)
{

}

/*===================================================
                仓道任务
====================================================*/
static struct etimer et_channel;
PROCESS(thread_channel, "通道任务");
AUTOSTART_PROCESSES(thread_channel);
PROCESS_THREAD(thread_channel, ev, data)  
{
	static U8 i = 0;
	static Channel *ch;
	static READ_TYPE_MODE mode;
	static U8 buf[13];
	PROCESS_BEGIN();
  channel_data_init();//初始化仓道数据
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++){
			ch = channel_data_get(i);
			if(ch==NULL)continue;
			
			if(isvalid_baibi() && isvalid_daowe() )//判断充电宝是否有效
			{		
				mode = iic_ir_select_poll(ch,FALSE);
        if(channel_read_busy(i,mode))continue;//忙，下一个仓道
				
				//读取ID
				if(!channel_read_start(i,mode,ch->iic_dir,RC_READ_ID))
					iic_ir_select_poll(ch,TRUE);//错误一次
				while(channel_read_busy(i,mode)){os_delay(et_channel,20);}//等待空闲
				while(channel_read_end(i,mode,buf)!=2){os_delay(et_channel,20);}//等待完成
				memcpy(ch->id,buf,10);
				
				//读取数据
				if(!channel_read_start(i,mode,ch->iic_dir,RC_READ_DATA))
				iic_ir_select_poll(ch,TRUE);//错误一次
				while(channel_read_busy(i,mode)){os_delay(et_channel,20);}//等待空闲
				while(channel_read_end(i,mode,buf)!=2){os_delay(et_channel,20);}//等待完成
				save_data(ch,buf);
				
				//6代以上，禁止输出,7代可以使用红外通讯
				
			}
			
		}
     os_delay(et_channel,2800);
	}

	PROCESS_END();
}
