#include "types.h"
#include "dev.h"

/*===================================================
                �����ļ�
====================================================*/

/*===================================================
                �꣬����
====================================================*/
/**************************************
	private ģ��i2c����
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
		return FALSE;	//SDA��Ϊ�͵�ƽ������æ,�˳�
	SDA_L(index);
	I2C_DELAY();
	if(SDA_READ(index) == TRUE) 
		return FALSE;	//SDA��Ϊ�ߵ�ƽ�����߳���,�˳�
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

static __inline BOOL I2C_WaitAck(U8 sda,U8 scl)	 //����Ϊ:=1��ACK,=0��ACK
{
	int c = 0;
	SCL_L(index);
	I2C_DELAY();
	SDA_H(index);			
	I2C_DELAY();
	SCL_H(index);
	I2C_DELAY();
	//�ȴ�sda ����
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

static __inline void I2C_SendByte(U8 sda,U8 scl,U8 ucData) //���ݴӸ�λ����λ//
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

static __inline U8 I2C_ReceiveByte(U8 sda,U8 scl)  //���ݴӸ�λ����λ//
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
                ˽��BQ27541����
====================================================*/
static 
/*===================================================
                ȫ�ֺ���
====================================================*/
//��ʼ�����õ�iic�ӿ�
void ld_iic_init(U8 sda,U8 scl)
{

}

/*
* ��ȡbq25741 id
* sda :sda �˿ں�
* scl :scl �˿ں�
* dataout: �������
*return  : TRUE or FALSE
*/
BOOL ld_bq25741_read_id(U8 sda,U8 scl,U8*dataout)
{
	


}

/*
* ��ȡbq25741 ����
* sda :sda �˿ں�
* scl :scl �˿ں�
* dataout: �������
*return  : TRUE or FALSE
*/
BOOL ld_bq25741_read_data(U8 sda,U8 scl,U8*dataout)
{
	


}