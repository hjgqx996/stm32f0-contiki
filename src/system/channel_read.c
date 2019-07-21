
/*===================================================
功能:仓道<====(iic,ir策略)=====>充电宝
1.策略包括:
  (1)如果指定iic,就只能使用iic通讯
	(2)如果指定ir,就只能使用ir通讯
	(3)两者都可以通讯时，出错几次后切换
	(4)iic单次出错几次后，切换方向，如果再错几次，就应该切换到ir

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
static U8 state[CHANNEL_MAX]={0};      //命令运行时的状态
static void*thread[CHANNEL_MAX]={0};   //命令运行时的线程,只支持单线程运行，不支持多线程并行
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
		mode = (ch->iic_ir_mode);
		if(mode==RTM_IR)        //当前:红外
		{
		  ch->iic_ir_mode_counter+=(error==TRUE)?1:0;
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
			ch->iic_dir_counter+=(error==TRUE)?1:0;
			if(ch->iic_dir_counter>=IIC_DIR_SWITCH_MAX)
			{
				ch->iic_dir_counter=0;
				ch->iic_dir=(ch->iic_dir+1)&0x01;
				ch->iic_ir_mode_counter++;
			}
		}
	}
	
	return ch->iic_ir_mode;
}
/*----------------------------------
充电宝操作重定向:是否忙,读 ,是否完成
-----------------------------------*/
static BOOL channel_read_busy(U8 ch,READ_TYPE_MODE mode)//是否忙
{
	if(mode == RTM_IIC)return ld_iic_busy(ch);
	if(mode == RTM_IR )return ld_ir_busy (ch);
	return FALSE;
}
static BOOL channel_read_cmd(U8 ch,U8 cmd,READ_TYPE_MODE mode)//当前命令是否cmd
{
	if(mode == RTM_IIC)return ld_iic_cmd(ch,cmd);
	if(mode == RTM_IR )return ld_ir_cmd(ch,cmd);
	return FALSE;
}
/*ch:1-n*/
static BOOL channel_read_start(U8 ch,READ_TYPE_CMD cmd,READ_TYPE_MODE mode,U8 dir)//开始命令
{
	U8 wanlen=0;//要读取的长度
	if(mode == RTM_IIC)
	{
		switch(cmd)
		{
			case RC_READ_ID:wanlen=10;break;
			case RC_READ_DATA:wanlen=13;break;
			case RC_OUTPUT:wanlen=1;break;
			case RC_LOCK:case RC_UNLOCK: case RC_UNLOCK_1HOUR: wanlen=1;break;
			default: return FALSE;
		}
		return ld_iic_read_start(ch,dir, cmd,wanlen);
	}
	else if(mode ==RTM_IR){
		switch(cmd)
		{
			case RC_READ_ID:wanlen=7;break;
			case RC_READ_DATA:wanlen=13;break;
			case RC_OUTPUT:return FALSE;
			case RC_LOCK:case RC_UNLOCK: case RC_UNLOCK_1HOUR: wanlen=2;break;
			default: return FALSE;
		}
		return ld_ir_read_start(ch,dir, cmd,wanlen);
	}
	else return FALSE;
}
/* 
* 查询命令是否运行结束
* return : <0：error
*        :  0: 无操作
*        :  1: 正在读
*        :  2: 读正确
*/
static int channel_read_end(U8 ch,U8*dataout,READ_TYPE_MODE mode)
{
	if(mode == RTM_IIC)
		return ld_iic_read_isok(ch,dataout,0);
	else if(mode ==RTM_IR)
		return ld_ir_read_isok(ch,dataout,0);
	else return 2;
}

/*===================================================
            全局函数

充电宝读==>非阻塞查询方式

pch:仓道数据
cmd:读命令
dataout:成功后返回的数据 

返回:  
	0:本命令未开始
	1:本命令在运行 
	2:本命令成功  
	3:本命令无法读取
====================================================*/
U8 channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout)
{
	#define s (state[ch-1])
	#define t (thread[ch-1])
	U8 ch = channel_data_get_index(pch);    //索引从 1 开始
	READ_TYPE_MODE mode = pch->iic_ir_mode; //iic ir模式   
	U8 dir = pch->iic_dir;                  //方向
	if(ch==0||pch==NULL)return 0;

	//其它线程正在使用
	if(channel_read_busy(ch,mode)&& (t!=PROCESS_CURRENT()))return 0;
	
	//命令未运行
	if(s==0)
	{
		//非本命令在运行
		if(channel_read_busy(ch,mode)) return s;
		else
		{
			if(channel_read_start(ch,cmd,mode,dir)==FALSE)
			{
				goto CHANNEL_READ_ERROR;
			}
			else {t=PROCESS_CURRENT();s =1;return 1;}
		}
	}
	//本命令已经开始
	if(s==1)
	{
		if( channel_read_busy(ch,mode) && channel_read_cmd(ch,cmd,mode) && (t==PROCESS_CURRENT())){return 1;}//本命令正在运行
		{
			int err= channel_read_end(ch,dataout,mode);
		  if(err<0)//本命令失败
			{
				goto CHANNEL_READ_ERROR;
			}          
		  else if(err==2)//本命令成功
			{
				s=0;
				t=0;
				 (pch,FALSE,TRUE);//计数清0
				return 2;
			} 
	  }
	}
	
	if(s>=2)
	{
		//超时
		if(s==4){			
			CHANNEL_READ_ERROR:
				iic_ir_select_poll(pch,TRUE,FALSE);//记错一次
			  t=0;
		}
		s=0;
		t=0;
    return 3;	
	}
}