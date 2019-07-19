#include "types.h"
#include "dev.h"
#include "string.h"
#include "lib.h"
/*===================================================
                配置文件
====================================================*/
#define sda_out()
/**************************************
	private 模拟i2c驱动
**************************************/
#define SCL_H()			ld_gpio_set(scl,HIGH)
#define SCL_L()			ld_gpio_set(scl,LOW)

#define SDA_H()			ld_gpio_set(sda,HIGH)
#define SDA_L()			ld_gpio_set(sda,LOW)

#define SDA_READ()		ld_gpio_get(sda)
#define SCL_READ()		ld_gpio_get(scl)

//extern void cpu_us_delay(int us);
//#define delayus(n)   cpu_us_delay(n)

#define i2c_start()     		I2C_Start(sda,scl)
#define i2c_restart()   		I2C_Restart(sda,scl)
#define i2c_stop()      		I2C_Stop(sda,scl)
#define i2c_ack()       		I2C_Ack(sda,scl)
#define i2c_noack()         I2C_NoAck(sda,scl)
#define i2c_wait_ack()      I2C_WaitAck(sda,scl)
#define i2c_send_byte(data) I2C_Send_Byte(sda,scl,data)
#define i2c_read_byte()     I2C_Read_Byte(sda,scl)
#define i2c_check_ack()     if(i2c_wait_ack()==FALSE) {i2c_stop();return FALSE;}



//判断时钟线是否为高
//不高则等待一定时间，大概950us左右
//2017-7-21
void wait_scl_high(U8 scl)
{
	U16 c=100; //100us超时
	while(c>0)
  {
		 delayus(1);
		 if(SCL_READ()==1){
			  c=c;
		    return ;
		 }
     c--;
  }
}

static void I2C_Start(U8 sda,U8 scl)
{
  sda_out();
	SDA_H();
	SCL_H();
 	delayus(5);	
//	wait_scl_high(scl);	
// 	delayus(5);
	SDA_L();
	delayus(5);
	SCL_L();
	delayus(5);
}

static void I2C_Restart(U8 sda,U8 scl)
{
  sda_out();
	SCL_L();
	delayus(5);
	SDA_H();
	delayus(5);
	SCL_H();
	wait_scl_high(scl);	
 	delayus(5);
	SDA_L();
	delayus(5);
	SCL_L();
	delayus(5);
}

static void I2C_Stop(U8 sda,U8 scl)
{
	sda_out();
	SDA_L();
	delayus(5);
	SCL_H();
	delayus(5);
	SDA_H();
	delayus(5);
}

static void I2C_Ack(U8 sda,U8 scl)
{	
	SDA_L();
	delayus(2);
	SCL_H();
	delayus(6);
	SCL_L();
	SDA_H();
	delayus(2);
}

static void I2C_NoAck(U8 sda,U8 scl)
{	
	SDA_H();
	delayus(5);
	SCL_H();
	delayus(5); //BQ2754延时5us
	SCL_L();
	SDA_L();
	delayus(5);
}

static BOOL I2C_WaitAck(U8 sda,U8 scl)	 //返回为:=TRUE有ACK,=FALSE无ACK
{
	U8 data=0;
	SDA_H();			
	SCL_H();
	delayus(5);
	wait_scl_high(scl);
	data=SDA_READ();
	SCL_L();
	delayus(5);
  return (data==1)?FALSE:TRUE;//sda==0,有应答
}

static void I2C_Send_Byte(U8 sda,U8 scl,U8 ucData) //数据从高位到低位//
{
	U8 i=0,temps,dat;
	temps=ucData;
	dat=0x80;
	for(i=0;i<8;i++)
	{
		if(dat&temps)
		{
			SDA_H();
			delayus(2);
			SCL_H();
			delayus(2);
			SCL_L();
			delayus(2);
		}else{
			SDA_L();
			delayus(2);
			SCL_H();
			delayus(2);
			SCL_L();
			delayus(2);
		}
		dat=dat>>1;
	}
	SDA_H();
	delayus(1);
}

static  U8 I2C_Read_Byte(U8 sda,U8 scl)  //数据从高位到低位//
{ 
	U8 i=8;
	U8 ucData=0;
	SDA_H();				
	while(i--)
	{
		ucData<<=1;      
		SCL_L();
		delayus(5);
		SCL_H();
		wait_scl_high(scl);
		delayus(5);	
		if(SDA_READ())
		{
			ucData|=0x01;
		}
	}
	SCL_L();
	delayus(4);
	return ucData;
}

/*===================================================
                私有BQ27541函数
====================================================*/
#define BQ27541_ADD_WR 		0xAA        //定义iic器件写地址
#define BQ27541_ADD_RD 		0xAB        //定义iic器件读地址

/*读一个字
* cmd: 命令字节，这里指寄存器地址
* dataout:返回的数据
*/
static BOOL bq27541_read_word(U8 sda,U8 scl,U8 cmd,U16 *dataout)
{
	U8 L,H;
	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack() //2017-7-21
	i2c_send_byte(cmd);
	i2c_check_ack() //2017-7-21
	i2c_restart();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack() //2017-7-21
	L=i2c_read_byte();//低字节
	i2c_ack();
	H=i2c_read_byte();//高字节
	i2c_noack();
	i2c_stop();
	*dataout=(((U16)H)<<8)|L;
	return TRUE;
}

/*读一个字节
* cmd: 命令字节，这里指寄存器地址
* dataout:返回的数据
*/
static BOOL bq27541_read_byte(U8 sda,U8 scl,U8 cmd,U8 *dataout)
{
	U8 temp;
	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack() //2017-7-21
	i2c_send_byte(cmd);
	i2c_check_ack() //2017-7-21
	i2c_restart();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack() //2017-7-21
	temp=i2c_read_byte();
	i2c_noack();
	i2c_stop();
	*dataout=temp;
	return TRUE;
}

//读取编号字节
static BOOL bq27541_read_power(U8 sda,U8 scl,U8*dataout)
{
	int i = 0;
	for(;i<10;i++)
	{
		if(bq27541_read_byte(sda,scl,(0x40+(9-i)),dataout)==FALSE)
			return FALSE;
		else
			dataout++;
	}
	return TRUE;
}

/*The bq27541 detects whether the SMBus enters the Off State by monitoring the SMBC and SMBD lines. When
both signals are continually low for at least 2.0 s, the bq27541 detects the Off State. When the SMBC and SMBD
lines go high, the bq27541 detects the On State and can begin communication within 1 ms. */
static void bq27541_smb_on(U8 sda,U8 scl)
{
	sda_out();
	SDA_H();
	SCL_H();
	delayus(5000); //不可取，建议外部延时
}

/*===================================================
                全局函数
====================================================*/

/*
* 读取bq25741 n个字
* sda :sda 端口号
* scl :scl 端口号
* cmd :命令数组
* cmdlen:命令数组长度
* dataout: 数据输出
*return  : TRUE or FALSE
*/
BOOL ld_bq27541_read_words(U8 sda,U8 scl,U8*cmd,U8 cmdlen,U16 *dataout)
{
	if(cmd==NULL||dataout==NULL||cmdlen>100)return FALSE;
	{
		int i=0;
		for(;i<cmdlen;i++)
		{
			if(bq27541_read_word(sda,scl,cmd[i],dataout)==FALSE)
				return FALSE;
			dataout++;
		}
	}
	return TRUE;
}

/*
* 读取bq25741 id
* sda :sda 端口号
* scl :scl 端口号
* dataout: 数据输出
*return  : TRUE or FALSE
*  把read_id分成三部分:
*  start----50ms----end
*  原因是50ms硬延时可能会产生严重后果
*/
BOOL ld_bq27541_read_id_start(U8 sda,U8 scl)
{
 	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x3F); 
  i2c_check_ack()
	i2c_send_byte(0x01); 
  i2c_check_ack()
	i2c_stop();
	return TRUE;
}
BOOL ld_bq27541_read_id_end(U8 sda,U8 scl,U8*dataout)
{
	U8 temp[10],cs,byte;
	//读取编号字节
	if(bq27541_read_power(sda,scl,temp)==FALSE)
	{
		i2c_stop();//其实这里已经在bq27541_read_power中处理
		return FALSE;
	}
	//检验
	cs = 0xFF-cs8(temp,10);
	if(cs!=0xFF){
		bq27541_read_byte(sda,scl,0x60,&byte);
		if(byte!=cs){
			//失败6次，充电宝编号 清0
			return FALSE;
		}
		else
			memcpy(dataout,temp,10);
	}else{
		memcpy(dataout,temp,10);
	}
	return TRUE;
}


/*
* 加密 解密 充电宝
* sda :sda 端口号
* scl :scl 端口号
* cmd :05:长解密 06:长加密 07:解密1小时
*return  : TRUE or FALSE
*  把read_id分成三部分:
*  start----50ms----end
*  原因是50ms硬延时可能会产生严重后果
*/
BOOL ld_bq27541_de_encrypt_charge_start(U8 sda,U8 scl,U8 cmd)
{
	U8 data[2];
 	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x62); 
  i2c_check_ack()
	i2c_stop();
	
	delayus(5);
	
	i2c_start();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack()
	data[0]= i2c_read_byte();
	i2c_ack();
	data[1]= i2c_read_byte(); 
	i2c_stop();
	
	delayus(5);

 	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x71); 
  i2c_check_ack()
	i2c_send_byte(cmd); 
  i2c_check_ack()
	i2c_stop();
	
	return TRUE;	
}
BOOL ld_bq27541_de_encrypt_charge_end(U8 sda,U8 scl)
{
	U8 data = 0;
	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x71); 
  i2c_check_ack()
	i2c_stop();
	
	delayus(5);
	
	i2c_start();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack()
	data= i2c_read_byte();
	i2c_noack();
	i2c_stop();
	return TRUE;
}


/*bq27541检测应答
*功能：开始读两次0x62，李工要求
*/
BOOL ld_bq27541_check_ack(U8 sda,U8 scl)
{
	
	U16 data = 0;
	if(bq27541_read_word(sda,scl,0x62,&data)==FALSE)return FALSE;
	if(data!=0x6207)return FALSE ;
	return TRUE;
}

/*bq27541充电宝输出标志*/
BOOL ld_bq27541_output_flag(U8 sda,U8 scl,U8*data)
{
	U16 tmp;
	if(bq27541_read_word(sda,scl,0x62,&tmp)==FALSE)return FALSE;
	if(bq27541_read_byte(sda,scl,0x71,(U8*)&tmp)==FALSE)return FALSE;
	*data=(U8)tmp;
	return TRUE;
}

/*===================================================
                标准化接口，像红外一样的接口
====================================================*/
#include "channel.h"
#define IIC_DATA_MAX      16
#define IIC_CHANNEL_MAX   5

/*状态*/
typedef enum{	
	IIC_State_Error=-1,
	IIC_State_NULL=0,     
	IIC_State_OK   =2,       //读取数据正确
}IIC_STATE;

typedef struct{
	U8 sda;        			//红外发送io
	U8 scl;        			//红外接收io
	BOOL opposite;      //反向
	READ_TYPE_CMD cmd;  //发送命令	
	U8 wanlen;       	  //要接收的数据长度
	U8 len;          		//实际接收到的数据长度
	BOOL start;      		//TRUE: 开始  FALSE:结束
	IIC_STATE state;    //错误码		
	S32 counter;     		//计数
	U16 data[IIC_DATA_MAX/2]; //接收数据
	U8 tmp;          			//缓存一字节
	
	BOOL inited;          //初始化标志，未初始化，状态机不能运行
	////////////////////////////////
	FSM fsm;         //状态机私有变量
}IIC_Type;
static IIC_Type iics[IIC_CHANNEL_MAX];
#define iic_lock()
#define iic_unlock()
#define sda_port()   ((piic->opposite)?(piic->scl):(piic->sda))
#define scl_port()   ((piic->opposite)?(piic->sda):(piic->scl))

const unsigned char IIC_DATA_CMDS[] ={//循环次数 /温度 /剩余容量  /电流
																		0x2a,    0x06, 0x04,      0x14,
};

/*
*  iic 状态机
*  初衷: 把iic中的50ms硬延时释放出来，让cpu可以做其它事情
*  接口实现:   ld_iic_read_start 启动状态机进行读
*              ld_iic_read_isok  查看状态机是否完成读操作
*  状态机实现:
           开始--->检测应答---->
              (1)读id           ld_bq27541_read_id_start---->延时50ms(状态机释放cpu)---->ld_bq27541_read_id_end
              (2)读数据         ld_bq27541_read_words
              (3)加解密         ld_bq27541_de_encrypt_charge_start---->延时50ms(状态机释放cpu)---->ld_bq27541_de_encrypt_charge_end
              (4)读输出         ld_bq27541_output_flag
*  想要运行状态机，必须不断重入，这里使用线程   AUTOSTART_THREAD_WITH_TIMEOUT(iic)
   来实现状态机的不断重入
*/
static void iic_fsm(IIC_Type*piic,FSM*fsm)
{
	U8 tmp=0;
	fsm_time_set(time(0));
	
	Start(开始)
	{
		if((piic==NULL) || (piic->inited==FALSE) || (piic->inited==FALSE))return;
		if(piic->start)
		{
			
			if(ld_bq27541_check_ack(sda_port(),scl_port()))//检测应答
			{
				switch(piic->cmd)
				{
					//读ic
					case RC_READ_ID:
						if(ld_bq27541_read_id_start(sda_port(),scl_port()))
						{goto 开始读id延时50ms;}
						else 
							goto IIC_FSM_Error;
						
					//读数据
					case RC_READ_DATA:
						if(ld_bq27541_read_words(sda_port(),scl_port(),(U8*)IIC_DATA_CMDS,4,piic->data))
						{ piic->len=8; goto IIC_FSM_Sucess;}
						else
							goto IIC_FSM_Error;
						
					//加解密	
					case RC_LOCK:if(tmp==0)tmp=0x06;
					case RC_UNLOCK:if(tmp==0)tmp=0x05;
					case RC_UNLOCK_1HOUR:if(tmp==0)tmp=0x07;
						if(ld_bq27541_de_encrypt_charge_start(sda_port(),scl_port(),tmp))
						{goto 开始加解密延时50ms;}
						else 
							goto IIC_FSM_Error;
						
					//输出标志
					case RC_OUTPUT:
						if(ld_bq27541_output_flag(sda_port(),scl_port(),(U8*)piic->data))
						{piic->len=1;goto IIC_FSM_Sucess;}
						else 
							goto IIC_FSM_Error;
					default:
						goto IIC_FSM_Error;
				}			
				
			}
			else
			{//检测应答失败
			  goto IIC_FSM_Error;
			}				
		}
	}
	
	State(开始读id延时50ms){waitmsx(50);goto 结束读id;}
	State(结束读id)
	{
	  if(ld_bq27541_read_id_end(sda_port(),scl_port(),(U8*)piic->data))
		{
			piic->len=13;
			goto IIC_FSM_Sucess;
		}
		else
			goto IIC_FSM_Error;
	}
	
	State(开始加解密延时50ms){waitmsx(50);goto 结束加解密;}
	State(结束加解密)
	{
		if(ld_bq27541_de_encrypt_charge_end(sda_port(),scl_port()))
		{
			if(ld_bq27541_output_flag(sda_port(),scl_port(),(U8*)piic->data))//读取标志
				goto IIC_FSM_Sucess;
			else 
				goto IIC_FSM_Error;
		}	
		else 
			goto IIC_FSM_Error;
	}
	
	Default()
	return ;
	
	IIC_FSM_Error:
	 piic->start=FALSE;
	 piic->state=IIC_State_Error;
	 memset(&piic->fsm,0,sizeof(FSM));
	 return;
	IIC_FSM_Sucess:
	 piic->start=FALSE;
	 piic->state=IIC_State_OK;
	 memset(&piic->fsm,0,sizeof(FSM));
	 return;
}

#include "contiki.h"
AUTOSTART_THREAD_WITH_TIMEOUT(iic)
{
	PROCESS_BEGIN();
	while(1)
	{
		static U8 i = 0;
		for(i=0;i<IIC_CHANNEL_MAX;i++)
		{
			if((iics[i].inited==TRUE) && (iics[i].start==TRUE))
			{
				iic_fsm(iics+i,&(iics+i)->fsm);
			}
		}
		os_delay(iic,10);
	}
	PROCESS_END();
}

/*===================================================
                全局函数
====================================================*/
/*初始化配置
* ch    :仓道号 1-n
* sda :数据端口
* scl :时钟端口
*/
void ld_iic_init(U8 ch,U8 sda,U8 scl)
{
	ch-=1;
	iic_lock();
	if(ch>=IIC_CHANNEL_MAX){
		iic_unlock();return;
	}
  memset(&iics[ch],0,sizeof(IIC_Type));
	iics[ch].sda=sda;
	iics[ch].scl=scl;
	iics[ch].start=FALSE;
	iics[ch].state=IIC_State_NULL;
	iics[ch].inited = TRUE;
	iic_unlock();
}
//启动读
BOOL ld_iic_read_start(U8 ch,BOOL opposite,U8 cmd,U8 wanlen)//(ch:1-n,opposite:TRUE反向, cmd 命令, 长度)
{
 	ch-=1;
	iic_lock();
	if( (ch>=IIC_CHANNEL_MAX) || (iics[ch].inited==FALSE) ){
		iic_unlock();return FALSE;
	}
	iics[ch].start=TRUE;
	iics[ch].state=IIC_State_NULL;
	iics[ch].cmd = (READ_TYPE_CMD)cmd;
	iics[ch].wanlen=wanlen;
	iics[ch].opposite=opposite;
	iic_unlock();

}
//是否忙
BOOL ld_iic_busy(U8 ch)
{
	BOOL r = 0;
	ch-=1;
	iic_lock();
	if(ch>=IIC_CHANNEL_MAX){iic_unlock();return FALSE;}
	r = iics[ch].start;
	iic_unlock();
	return r;
}
//是否命令
BOOL ld_iic_cmd(U8 ch,U8 cmd)
{
	ch-=1;
	iic_lock();
	if(ch>=IIC_CHANNEL_MAX){iic_unlock();return FALSE;}
	iic_unlock();
	return (cmd==iics[ch].cmd);
}

/*查看是否读完成
* return : <0：error
*        :  0: 无操作
*        :  1: 正在读
*        :  2: 读正确
*/
int ld_iic_read_isok(U8 ch,U8*dataout,U8 size)
{
	int err = -1;
	ch-=1;
  iic_lock();
	if(ch>=IIC_CHANNEL_MAX)goto END;
	if(iics[ch].start==FALSE)
	{
		err=(int)iics[ch].state;
	}else{
		err = 1;
	}
	//正确时，弹出数据:格式化输出
	if(err==IIC_State_OK && dataout!= NULL)
	{
		//格式化红外数据输出
		switch(iics[ch].cmd)
		{
			case RC_READ_ID:
				memcpy(dataout,iics[ch].data,10);
	    break;
			
			case RC_READ_DATA://[0] 版本号 [1] 电量 [2] 温度 [3] 故障码 [4-5] 循环次数 [6-7] 容量 [8-9] 电芯电压 [10-11] 电流 (低位在前)
			{
				#define dat   ((U8*)(iics[ch].data))
				#define dat16 iics[ch].data
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
			}break;
			
			case RC_LOCK:         
			case RC_UNLOCK:
			case RC_UNLOCK_1HOUR:
			case RC_OUTPUT:      //[0]输出标志
				dataout[0] = dat[0];
			break;
      default:goto END;
		}
	}
	END:
	iic_unlock();
	return err;
}

///*===================================================
//                IIC测试
//====================================================*/
//AUTOSTART_THREAD_WITH_TIMEOUT(iic_test)
//{
//	U8 dataout[13];
//	PROCESS_BEGIN();
//	while(1)
//	{
//		
//		ld_iic_read_start(2,FALSE,RC_READ_ID,10);
//		os_delay(iic_test,200);
//		ld_iic_read_isok(2,dataout,10);
//		os_delay(iic_test,10);
//		
////		ld_iic_read_start(2,FALSE,RC_READ_DATA,13);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,13);
////    os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_LOCK,0);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,0);
////		os_delay(iic_test,0);
////		
////		ld_iic_read_start(2,FALSE,RC_OUTPUT,1);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,1);
////		os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_UNLOCK,0);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,0);
////		os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_OUTPUT,1);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,1);
////		os_delay(iic_test,10);

////		ld_iic_read_start(2,FALSE,RC_UNLOCK_1HOUR,0);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,0);
////		os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_OUTPUT,1);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,1);
////		os_delay(iic_test,10);

//	}
//	PROCESS_END();
//}
