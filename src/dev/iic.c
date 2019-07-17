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

#define delayus(n)   cpu_us_delay(n)

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
	i2c_send_byte(0x01); 
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


/*===================================================
                标准化接口，像红外一样的接口
====================================================*/
typedef struct{
	U8 io_ir;        			//红外发送io
	U8 io_re;        			//红外接收io
	U8 cmd;          			//发送命令	
	U8 wanlen;       			//要接收的数据长度
	U8 len;          			//实际接收到的数据长度
	BOOL start;      			//TRUE: 开始  FALSE:结束
	S8 state;        			//错误码		
	S32 counter;     			//计数
	U8 data[IR_DATA_MAX]; //接收数据
	U8 tmp;          			//缓存一字节
	
	BOOL inited;          //初始化标志，未初始化，状态机不能运行
	////////////////////////////////
	FSM fsm;         //状态机私有变量
}IR_Type;

void ld_iic_init(U8 ch,U8 sda,U8 scl)
{
	

}
BOOL ld_iic_read_start(U8 ch,BOOL opposite,U8 cmd,U8 wanlen)//(ch:1-n,opposite:TRUE反向, cmd 命令, 长度)
{


}
int ld_iic_read_isok(U8 ch,U8*dataout,U8 size)
{

}

