
/*===================================================
功能:仓道<====(iic,ir策略)=====>充电宝
1.策略包括:
			(1)如果指定iic,就只能使用iic通讯
			(2)如果指定ir,就只能使用ir通讯
			(3)两者都可以通讯时，出错几次后切换
			(4)在使用iic时:出错几次后，切换方向，如果再错几次，就应该切换到ir

2.功能包括:
			(1)读id
			(2)读数据
			(3)解锁，上锁，解锁1小时,检查输出标志	
3.关于如何使用接口:
  while(channel_read(ch_index,cmd,dataout,timeout)==FALSE){ osdelay(...)}
  等待的时候，cpu被切换，不会影响其它程序的运行	
====================================================*/
#include "includes.h"

/*===================================================
                本地变量
====================================================*/
const unsigned char TESTDATA1[] =
{
0x2a,      0x06, 0x04,           0x14,
//循环次数 /温度 /剩余容量           /电流
};

/*===================================================
                本地函数
====================================================*/
/* 选择策略
*  ch: 仓道数据
*  error:通道是否出错，出错时为TRUE  (当读失败时，记错一次)
*  clear:复位计数
*  返回 :当前通讯方式(iic or ir)
*/
static READ_TYPE_MODE iic_ir_select_poll(Channel*ch,BOOL error,BOOL clear)
{
	READ_TYPE_MODE mode;
	
	if(clear==TRUE)//计数清0
	{
		ch->iic_dir_counter=0;
		ch->iic_ir_mode_counter=0;
	}

  if(system.iic_ir_mode==SIIM_ONLY_IR) {return RTM_IR;}	                   //强制使用 ir		
	if(system.iic_ir_mode==SIIM_ONLY_IIC){mode = RTM_IIC;goto MODE_IIC_POLL;}//强制使用 iic	
	
	if(system.iic_ir_mode==SIIM_IIC_IR)                                      //两者都可以用
	{
		mode = (READ_TYPE_MODE)(ch->iic_ir_mode);
		if(mode==RTM_IR)        //当前:红外
		{
			if(error==FALSE)
			{
				ch->iic_ir_mode_counter = 0;
			}
		  else 
				ch->iic_ir_mode_counter++;
			if(ch->iic_ir_mode_counter>=IR_TO_IIC_SWITCH_ERROR_MAX)//切换错误累加
			{
				ch->iic_ir_mode_counter=0;
				ch->iic_dir_counter=0;
				ch->iic_ir_mode=RTM_IIC;
			}
		}
		else if(mode==RTM_IIC) //当前:IIC
		{
			if(ch->iic_ir_mode_counter>=IIC_TO_IR_SWITCH_ERROR_MAX*2)//切换错误累加
			{
				ch->iic_ir_mode_counter=0;
				ch->iic_dir_counter=0;
				ch->iic_ir_mode=RTM_IR;
			}
			
			MODE_IIC_POLL:      //IICC通讯时，失败后方向改变
			if(error==FALSE)
			{
				ch->iic_ir_mode_counter=0;
			  ch->iic_dir_counter=0;
			}
			else ch->iic_dir_counter+=(error==TRUE)?1:0;
			if(ch->iic_dir_counter>=IIC_DIR_SWITCH_MAX)
			{
				ch->iic_dir_counter=0;
				ch->iic_dir=(ch->iic_dir+1)&0x01;
				ch->iic_ir_mode_counter++;
			}
		}
	}
	
	return (READ_TYPE_MODE)ch->iic_ir_mode;
}
/*===================================================
            全局函数
充电宝读==>非阻塞查询方式
pch:仓道数据
cmd:读命令
dataout:成功后返回的数据 
ms_timeout:超时ms
once:读一次(iic ir),前提是system.iic_ir_mode允许
return :TRUE or FALSE   -1:红外re高等待
====================================================*/
int channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout,int ms_timeout,BOOL once)
{
	U8 iic_cmd = 0;//红外与iic的加密 解密 指令是不一样的
	U16 buffer[8];
	U8 ch = channel_data_get_index(pch);    //索引从 1 开始
	READ_TYPE_MODE mode = (READ_TYPE_MODE)pch->iic_ir_mode; //iic ir模式   
	U8 dir = pch->iic_dir; 	//方向
	U8 sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;
	U8 scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;
	int result = 0;
	int wanlen = 0;
	if(ch==0||pch==NULL)return FALSE;
	
	
	if(system.iic_ir_mode==SIIM_ONLY_IR)mode=RTM_IR;
	if(system.iic_ir_mode==SIIM_ONLY_IIC)mode=RTM_IIC;
	
	if(mode == RTM_IIC || (once&&system.iic_ir_mode!=SIIM_ONLY_IR) )//是否允许
	{
		//没有应答改变方向再读一次
		if(ld_bq27541_check_ack(sda,scl)==FALSE)
		{
			if(ld_bq27541_check_ack(sda,scl)==FALSE)
			{
				//改变方向
				pch->iic_dir=!pch->iic_dir;
				dir = pch->iic_dir;//方向
				sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;
				scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;		
				if(ld_bq27541_check_ack(sda,scl)==FALSE)
					if(ld_bq27541_check_ack(sda,scl)==FALSE){
						if(once==FALSE)
						{
							iic_ir_select_poll(pch,TRUE,FALSE);//失败一次
							pch->dingzhen_counter++;           //顶针识别计数++
						}
						else 
							 if(system.iic_ir_mode!=SIIM_ONLY_IIC)
								 goto READ_IR;//读一次ir
						return FALSE;
					}		
			}
		}

		switch(cmd)
		{
			case RC_READ_ID:   result = ld_bq27541_read_id(sda,scl,dataout);
												 if(result==TRUE)memcpy(pch->id,dataout,CHANNEL_ID_MAX);
												
			break;
			case RC_READ_DATA: result = ld_bq27541_read_words(sda,scl,(U8*)TESTDATA1,4,buffer);
												 if(result==TRUE)//格式化输出
													{
														#define dat   ((U8*)(buffer))
														#define dat16 buffer
														U8 cs = 0xFF-cs8(dat,12);
														dataout[0]=0;
														dataout[1]=(U8)dat16[2];
														dataout[2]=(dat16[1]-2732)/10;
														dataout[3]=0;
														dataout[4]=dat[0];
														dataout[5]=dat[1];//循环次数
														dataout[6]=dataout[7]=0;//容量
														dataout[8]=dataout[9]=0;//电压
														dataout[10]=dat[6];
														dataout[11]=dat[7];
														channel_save_data(pch,dataout);
													}	
													
			break;
			case RC_UNLOCK:if(iic_cmd==0)      iic_cmd=BAO_ALLOW;               //iic指令 解      05
		  case RC_UNLOCK_1HOUR:if(iic_cmd==0)iic_cmd=BAO_ALLOW_ONE_HOUR;//iic指令 解1小时 07
			case RC_LOCK:if(iic_cmd==0)        iic_cmd=BAO_NOTALLOW;              //iic指令 不输出  06
			 result = ld_bq27541_de_encrypt_charge(sda,scl,iic_cmd);
			 if(result==TRUE)
				 result =ld_bq27541_output_flag(sda,scl,dataout);
			 break;
			default:return FALSE;		 									
		}
		
		if(result==TRUE){pch->dingzhen_counter=0;}                //顶针识别清0
		
		if(once==FALSE)
			iic_ir_select_poll(pch,(result==TRUE?FALSE:TRUE),FALSE);//记录一次，是否要切换
		else 
		{
			if(result==FALSE && (system.iic_ir_mode!=SIIM_ONLY_IIC))//是否允许再读一次红外
			 goto READ_IR;//读一次ir
		}
		return result;
	}
	else if(mode==RTM_IR)
	{
		
		if(ld_gpio_get(pch->map->io_re))return -1;
		
		READ_IR:
		pch->iic_ir_mode=RTM_IR;
		switch(cmd)
		{
			case RC_READ_ID  :wanlen =  7;break;  //实测406ms
			case RC_READ_DATA:wanlen = 13;break;  //实测504ms
			default:wanlen = 2;
		}
		//启动命令
	  ld_ir_read_start(ch,0,cmd,wanlen);
		//等待结束 
		while((result=ld_ir_read_isok(ch,(U8*)(buffer),wanlen))==1)
		{
			delayms(10);
			ms_timeout-=10;
			if(ms_timeout<0){
				result = 4;
				return FALSE;
			}
		}

		//输出结果
	  if(result==2)
		{
			if(cmd==RC_READ_ID)  memcpy(pch->id,(U8*)(buffer),10);
			if(cmd==RC_READ_DATA)channel_save_data(pch,(U8*)(buffer));
			iic_ir_select_poll(pch,FALSE,FALSE);
			memcpy(dataout,(U8*)buffer,13);
			pch->ir_error_counter=0;
			delayms(10);//成功后延时10ms
			return TRUE;
		}
		//失败后延时80ms
		else{
			delayms(120);
		}
		
		//红外识别故障++
		pch->ir_error_counter++;
		{
			U8 pr[5]={ch};
			memcpy(pr+1,&pch->ir_error_counter,4);		
			enable_485_tx();;
		  ld_uart_send(COM_485,pr,5);
		}
		iic_ir_select_poll(pch,TRUE,FALSE);
		return FALSE;
	}
	
	return result;
}


