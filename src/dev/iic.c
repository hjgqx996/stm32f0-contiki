#include "types.h"
#include "dev.h"

/*===================================================
                配置文件
====================================================*/

/*===================================================
                宏，类型
====================================================*/
/**************************************
	private 模拟i2c驱动
**************************************/
#define SCL_H(N)			ld_gpio_set(scl,HIGH)
#define SCL_L(N)			ld_gpio_set(scl,LOW)

#define SDA_H(N)			ld_gpio_set(sda,HIGH)
#define SDA_L(N)			ld_gpio_set(sda,LOW)

#define SDA_READ(N)		ld_gpio_get(sda)
#define SCL_READ(N)		ld_gpio_get(scl)

#define I2C_DELAY()   cpu_us_delay(5)

static __inline BOOL I2C_Start(U8 sda,U8 scl)
{
	SDA_H(index);
	SCL_H(index);
	I2C_DELAY();
	if(SDA_READ(index)!= TRUE)
		return FALSE;	//SDA线为低电平则总线忙,退出
	SDA_L(index);
	I2C_DELAY();
	if(SDA_READ(index) == TRUE) 
		return FALSE;	//SDA线为高电平则总线出错,退出
	SDA_L(index);
	I2C_DELAY();
	return TRUE;
}

static __inline void I2C_Stop(U8 sda,U8 scl)
{
	SCL_L(index);
	I2C_DELAY();
	SDA_L(index);
	I2C_DELAY();
	SCL_H(index);
	I2C_DELAY();
	SDA_H(index);
	I2C_DELAY();
}

static __inline void I2C_Ack(U8 sda,U8 scl)
{	
	SCL_L(index);
	I2C_DELAY();
	SDA_L(index);
	I2C_DELAY();
	SCL_H(index);
	I2C_DELAY();
	SCL_L(index);
	I2C_DELAY();
}

static __inline void I2C_NoAck(U8 sda,U8 scl)
{	
	SCL_L(index);
	I2C_DELAY();
	SDA_H(index);
	I2C_DELAY();
	SCL_H(index);
	I2C_DELAY();
	SCL_L(index);
	I2C_DELAY();
}

static __inline BOOL I2C_WaitAck(U8 sda,U8 scl)	 //返回为:=1有ACK,=0无ACK
{
	int c = 0;
	SCL_L(index);
	I2C_DELAY();
	SDA_H(index);			
	I2C_DELAY();
	SCL_H(index);
	I2C_DELAY();
	//等待sda 拉低
	while(SDA_READ(index))
	{
		c++;
		if(c>10000){
      SCL_L(index);
      return FALSE;
		}
	}
	SCL_L(index);
	return TRUE;
}

static __inline void I2C_SendByte(U8 sda,U8 scl,U8 ucData) //数据从高位到低位//
{
	U8 i=8;
	while(i--)
	{
		SCL_L(index);
		I2C_DELAY();
		if(ucData&0x80)
		{
			SDA_H(index); 
		}
		else 
		{
			SDA_L(index);
		}
		ucData<<=1;
		I2C_DELAY();
		SCL_H(index);
		I2C_DELAY();

	}
	SCL_L(index);
}

static __inline U8 I2C_ReceiveByte(U8 sda,U8 scl)  //数据从高位到低位//
{ 
	U8 i=8;
	U8 ucData=0;

	SDA_H(index);				
	while(i--)
	{
		ucData<<=1;      
		SCL_L(index);
		I2C_DELAY();
		SCL_H(index);
		I2C_DELAY();	
		if(SDA_READ(index))
		{
			ucData|=0x01;
		}
	}
	SCL_L(index);
	return ucData;
}

/*===================================================
                私有BQ27541函数
====================================================*/
static 
/*===================================================
                全局函数
====================================================*/
//初始化配置的iic接口
void ld_iic_init(U8 sda,U8 scl)
{

}

/*
* 读取bq25741 id
* sda :sda 端口号
* scl :scl 端口号
* dataout: 数据输出
*return  : TRUE or FALSE
*/
BOOL ld_bq25741_read_id(U8 sda,U8 scl,U8*dataout)
{
	


}

/*
* 读取bq25741 数据
* sda :sda 端口号
* scl :scl 端口号
* dataout: 数据输出
*return  : TRUE or FALSE
*/
BOOL ld_bq25741_read_data(U8 sda,U8 scl,U8*dataout)
{
	


}