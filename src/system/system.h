#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"
#include "config.h"


/*系统租借状态*/
typedef enum{
	LEASE_STATE_INIT = 0, //初始态
	LEASE_STATE_LEASE,    //租借态
	LEASE_STATE_RETURN,   //归还态
	LEASE_STATE_CTRL,     //控制态

}LeaseState;

/*通讯方式*/
typedef enum{
	SIIM_ONLY_IIC=1,   //只使用IIC
	SIIM_ONLY_IR,      //只使用IR
	SIIM_IIC_IR,       //IIC,IR同时使用
}SYSTEM_IIC_IR_MODE;


/*系统数据*/
typedef struct{
	
	volatile U8 addr485;                 					//485地址
	volatile U8 addr_ch[CHANNEL_MAX];    					//通道地址
	volatile SYSTEM_IIC_IR_MODE iic_ir_mode;      //通讯方式
	
	volatile LeaseState state;  //系统租借状态

	volatile	 U32    firmware_size;
	volatile	 U16    firmware_version;
	volatile	 U16    software_version;
	volatile	 U8     firmware_MD5[16];
	                 U8     firmware_updata_flag;   //固件更新标志位  0x88为更新状态
}System;


extern System system;

#endif
