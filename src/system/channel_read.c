
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
            全局函数
充电宝读==>阻塞查询方式
pch:仓道数据
cmd:读命令
dataout:成功后返回的数据 
ms_timeout:超时ms
once:=true:读iic 如果失败再读一次ir ,前提是system.iic_ir_mode允许
return :TRUE or FALSE   -1:红外re高等待
====================================================*/
int channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout,int ms_timeout,BOOL once)
{
	U8 iic_cmd = 0;//红外与iic的加密 解密 指令是不一样的
	U16 buffer[8];
	U8 ch = channel_data_get_index(pch);    //索引从 1 开始
	READ_TYPE_MODE mode = (READ_TYPE_MODE)pch->iic_ir_mode; //iic ir模式   
	int result = 0;
	int wanlen = 0;
	if(ch==0||pch==NULL)return FALSE;
	
	
	if(system.iic_ir_mode==SIIM_ONLY_IR)mode=RTM_IR;
	if(system.iic_ir_mode==SIIM_ONLY_IIC)mode=RTM_IIC;
	
	/*------------------------iic 方式 读取 -----------------------------------*/
	if(mode == RTM_IIC || (once&&system.iic_ir_mode!=SIIM_ONLY_IR) )//是否允许
	{
		U8 dir = pch->iic_dir; 	//方向
		U8 sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;
		U8 scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;
		//检测应答,尝试两次
		if((result=ld_bq27541_check_ack(sda,scl))==FALSE)
		{
			if((result=ld_bq27541_check_ack(sda,scl))==FALSE)
			{	
				/*改变方向，检测应答，尝试两次*/
				pch->iic_dir=!pch->iic_dir;//改变方向
				dir = pch->iic_dir;        //方向
				sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;//sda
				scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;//scl		
				if((result=ld_bq27541_check_ack(sda,scl))==FALSE)        
					if((result=ld_bq27541_check_ack(sda,scl))==FALSE){ 
						goto IIC_READ_ERROR;//读失败 
					}		
			}
		}
    //IIC读
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
			case RC_UNLOCK:if(iic_cmd==0)      iic_cmd=BAO_ALLOW;         //iic指令 解      05
		  case RC_UNLOCK_1HOUR:if(iic_cmd==0)iic_cmd=BAO_ALLOW_ONE_HOUR;//iic指令 解1小时 07
			case RC_LOCK:if(iic_cmd==0)        iic_cmd=BAO_NOTALLOW;      //iic指令 不输出  06
			 result = ld_bq27541_de_encrypt_charge(sda,scl,iic_cmd);
			 if(result==TRUE)
				 result =ld_bq27541_output_flag(sda,scl,dataout);           //读输出标志
			 break;
			default:return FALSE;		 									
		}
		/*iic 读失败*/
		IIC_READ_ERROR:
		if(result==FALSE){
			
			U8 pb[8];pb[0]=RTM_IIC; pb[1]=pch->addr;
			enable_485_tx();//使能发送
			ld_uart_send(COM_485,pb,2);//打印结果
			
			pch->iic_error_counter++;                              //顶针识别计数++
			if( (once==TRUE && system.iic_ir_mode!=SIIM_ONLY_IIC)  //可以读一次红外
				||(pch->iic_error_counter>=BAO_DINGZHEN_ERROR_TIMES))//3次iic错误，转红外
			{
				goto READ_IR;                //读一次ir
			}
			return FALSE;
		}else
			pch->iic_error_counter=0;
		
		return result;
	}
	
	/*------------------------红外 方式 读取 -----------------------------------*/
	else if(mode==RTM_IR)
	{
		if(ld_gpio_get(pch->map->io_re))return -1;/*红外被拉高，表示红外忙，直接返回-1*/
		
		READ_IR:
		pch->iic_ir_mode=RTM_IR;
		switch(cmd)
		{
			case RC_READ_ID  :wanlen =  7;break;  //实测406ms
			case RC_READ_DATA:wanlen = 13;break;  //实测504ms
			default:wanlen = 2;
		}
		
	  ld_ir_read_start(ch,FALSE,cmd,wanlen);  //启动命令
		
		while((result=ld_ir_read_isok(ch,(U8*)(buffer),wanlen))==1)//等待结束,超时时间ms_timeout ms
		{
			delayms(13);
			ms_timeout-=10;
			if(ms_timeout<0){
				result = 4;
				return FALSE;
			}
		}

		//输出结果
	  if(result==2)
		{
			if(cmd==RC_READ_ID)  memcpy(pch->id,(U8*)(buffer),10);     // copy id
			if(cmd==RC_READ_DATA)channel_save_data(pch,(U8*)(buffer)); // copy data
			memcpy(dataout,(U8*)buffer,2);                             //其它数据
			pch->ir_error_counter=0;                                   //红外错误标志清
			delayms(10);                                               //成功后延时10ms
			return TRUE;
		}
		else
		{
			delayms(120);                                              //失败后延时120ms
		}
		
		//红外识别故障++
		pch->ir_error_counter++;
		
		/*失败两次转iic*/
		if(pch->ir_error_counter>=BAO_IR_ERROR_TIMES){
			pch->iic_ir_mode=RTM_IIC;
		}
		
		{
			U8 pb[8];pb[0]=RTM_IR; pb[1]=pch->addr;
			enable_485_tx();//使能发送
			ld_uart_send(COM_485,pb,2);//打印结果
		}
		return FALSE;
	}
	
	return result;
}


