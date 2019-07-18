#include "includes.h"


/*===================================================
               宏
====================================================*/

#define isvalid_daowe()  ld_gpio_get(ch->map->io_detect) //到位开关有效
#define isvalid_baibi()  ld_gpio_get(ch->map->io_sw)     //摆臂开关有效
#define is_ver_6()       ((ch->id[6]&0x0F)==0x06)        //6代宝
#define is_ver_7()       ((ch->id[6]&0x0F)==0x07)        //7代宝
#define is_ver_lte_5()   ((ch->id[6]&0x0F)==0x05)        //5代或以下
#define is_output()      ((ch->


//等待空闲
#define channel_read_wait()         do{while(channel_read_busy(i,mode)){os_delay(channel,10);}}while(0) 	
//读一个仓道的一组数据,x为成功后要执行的代码
#define channel_read_start_wait_end(cmd,x)   \
				/*开始命令,出错时记错*/   if(!channel_read_start(i,mode,ch->iic_dir,cmd)){mode=iic_ir_select_poll(ch,TRUE);} \
				/*等待完成*/							else{ while(channel_read_busy(i,mode)){os_delay(channel,10);} \
				/*判断结果*/              if(channel_read_end(i,mode,buf)!=2){mode=iic_ir_select_poll(ch,TRUE);} \
				/*结果正确时处理x*/       else{x;}}
	
/*===================================================
                私有函数
====================================================*/

//错误计数，判断选择哪种方式来通讯==>用来记错，并返回当前选择的通讯方式
static READ_TYPE_MODE iic_ir_select_poll(Channel*ch,BOOL error)
{
	READ_TYPE_MODE mode;
  if(system.iic_ir_mode==SIIM_ONLY_IR) {return RTM_IR;}	//强制使用 ir		
	if(system.iic_ir_mode==SIIM_ONLY_IIC){mode = RTM_IIC;goto READ_TYPE_MODE_IIC;}//强制使用 iic	
	
	READ_TYPE_MODE_IIC_IR://方式切换
  mode = (ch->iic_ir_mode);
	if(ch->iic_ir_mode_counter>=IIC_IR_SWITCH_ERROR_MAX)//切换
	{
		ch->iic_ir_mode=0x01&(!ch->iic_ir_mode);
		ch->iic_dir_counter=ch->iic_ir_mode_counter=0;
		mode = ch->iic_ir_mode;
		return mode;
	}
	if(mode==RTM_IR)
	{
		ch->iic_ir_mode_counter+=(error==TRUE)?1:0;//切换错误累加
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
/*保存数据*///[0] 版本号 [1] 电量 [2] 温度 [3] 故障码 [4-5] 循环次数 [6-7] 容量 [8-9] 电芯电压 [10-11] 电流 (低位在前)
static void save_data(Channel*ch,U8*data)
{
	ch->Ver 							= data[0];
	ch->Ufsoc							=	data[1];
	ch->Temperature				=	data[2];
	ch->CycleCount				= (((U16)data[5])<<8)|(data[4]);
	ch->RemainingCapacity	= (((U16)data[7])<<8)|(data[6]);
	ch->Voltage          	= (((U16)data[9])<<8)|(data[8]);
	ch->AverageCurrent    = (((U16)data[11])<<8)|(data[10]);
}

/*===================================================
               仓道任务
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	static U8 i = 0;
	static Channel *ch;
	static READ_TYPE_MODE mode;
	static U8 buf[13];
	PROCESS_BEGIN();
  channel_data_init();//初始化仓道数据
	while(1)
	{
		/*--------------------------------------------------------
		1.对所有通道检查，看充电宝是否有效
		2.有效的充电宝执行以下操作:
		  (1) 读ID 
		  (2) 读数据
		  (3) 读输出标志
		  (4) 6代宝以上,非租借条件下，应该 加密
		3.执行频率 2.8秒
		---------------------------------------------------------*/	
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			ch = channel_data_get(i);
			if(ch==NULL)continue;
			
			if(isvalid_baibi() && isvalid_daowe() )															//判断充电宝是否有效
			{		
				mode = iic_ir_select_poll(ch,FALSE);  														//选择iic还是ir来通讯
        if(channel_read_busy(i,mode))continue;														//忙，下一个仓道
				
				channel_read_start_wait_end(RC_READ_ID,memcpy(ch->id,buf,10));		//读取ID
				
				channel_read_start_wait_end(RC_READ_DATA, save_data(ch,buf));			//读取数据
				
				if(mode==RTM_IIC)
					{channel_read_start_wait_end(RC_OUTPUT,ch->bao_output=buf[0]);}	//如果使用iic,要读取输出标志
				
				//非租借条件下(6代以上，禁止输出,7代可以使用红外通讯禁止输出)
			}
		}
    os_delay(channel,2800);
	}
	PROCESS_END();
}
