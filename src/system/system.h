#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"
#include "config.h"


/*ϵͳ���״̬*/
typedef enum{
	SYSTEM_STATE_INIT = 0, //��ʼ̬
	SYSTEM_STATE_LEASE,    //���̬
	SYSTEM_STATE_RETURN,   //�黹̬
	SYSTEM_STATE_CTRL,     //����̬
}SYSTEM_STATE;

/*ͨѶ��ʽ*/
typedef enum{
	SIIM_ONLY_IIC=1,   //ֻʹ��IIC
	SIIM_ONLY_IR,      //ֻʹ��IR
	SIIM_IIC_IR,       //IIC,IRͬʱʹ��
}SYSTEM_IIC_IR_MODE;

#pragma pack(1)
/*ϵͳ����*/
typedef struct{
	
	volatile U8 addr485;                 					//485��ַ
	volatile U8 addr_ch[CHANNEL_MAX];    					//ͨ����ַ
	volatile SYSTEM_IIC_IR_MODE iic_ir_mode;      //ͨѶ��ʽ
	
  volatile SYSTEM_STATE state;                  //ϵͳ���״̬

	volatile	 U32    firmware_size;
	volatile	 U16    firmware_version;
	volatile	 U16    software_version;
	volatile	 U8     firmware_MD5[16];
	           U8     firmware_updata_flag;       //�̼����±�־λ  0x88Ϊ����״̬
	
	/*------------ϵͳ�� led 0 ��˸����----------------*/
	int led_flash_mode;     //1: 100ms  2:500ms  3:2000ms
	int led_flash_time;    //ms
	
}System;

#pragma pack()
extern System system;



void ld_system_init(void);          /*ϵͳ���ݳ�ʼ��*/
void ld_system_flash_led(int ms);  /*����ϵͳ����˸Ƶ��*/
void ld_system_led_timer(int ms);  /*ϵͳ�ƶ�ʱ������*/


BOOL is_system_lease(void);//�Ƿ����

#endif
