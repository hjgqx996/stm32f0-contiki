#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"
#include "config.h"


/*ϵͳ���״̬*/
typedef enum{
	LEASE_STATE_INIT = 0, //��ʼ̬
	LEASE_STATE_LEASE,    //���̬
	LEASE_STATE_RETURN,   //�黹̬
	LEASE_STATE_CTRL,     //����̬

}LeaseState;

/*ͨѶ��ʽ*/
typedef enum{
	SIIM_ONLY_IIC=1,   //ֻʹ��IIC
	SIIM_ONLY_IR,      //ֻʹ��IR
	SIIM_IIC_IR,       //IIC,IRͬʱʹ��
}SYSTEM_IIC_IR_MODE;


/*ϵͳ����*/
typedef struct{
	volatile U8 addr485;                 					//485��ַ
	volatile U8 addr_ch[CHANNEL_MAX];    					//ͨ����ַ
	volatile SYSTEM_IIC_IR_MODE iic_ir_mode;      //ͨѶ��ʽ
}System;

extern System system;

#endif
