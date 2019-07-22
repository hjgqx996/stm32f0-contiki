#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"
#include "config.h"


/*系统租借状态*/
typedef enum{
	SYSTEM_STATE_INIT = 0, //初始态
	SYSTEM_STATE_LEASE,    //租借态
	SYSTEM_STATE_RETURN,   //归还态
	SYSTEM_STATE_CTRL,     //控制态
}SYSTEM_STATE;

/*通讯方式*/
typedef enum{
	SIIM_ONLY_IIC=1,   //只使用IIC
	SIIM_ONLY_IR,      //只使用IR
	SIIM_IIC_IR,       //IIC,IR同时使用
}SYSTEM_IIC_IR_MODE;

#pragma pack(1)
/*系统数据*/
typedef struct{
	
	volatile U8 addr485;                 					//485地址
	volatile U8 addr_ch[CHANNEL_MAX];    					//通道地址
	volatile SYSTEM_IIC_IR_MODE iic_ir_mode;      //通讯方式
	
  volatile SYSTEM_STATE state;                  //系统租借状态

	volatile	 U32    firmware_size;
	volatile	 U16    firmware_version;
	volatile	 U16    software_version;
	volatile	 U8     firmware_MD5[16];
	           U8     firmware_updata_flag;       //固件更新标志位  0x88为更新状态
	
	/*------------系统灯 led 0 闪烁控制----------------*/
	int led_flash_mode;     //1: 100ms  2:500ms  3:2000ms
	int led_flash_time;    //ms
	
}System;

#pragma pack()
extern System system;



void ld_system_init(void);          /*系统数据初始化*/
void ld_system_flash_led(int ms);  /*设置系统灯闪烁频率*/
void ld_system_led_timer(int ms);  /*系统灯定时器处理*/


BOOL is_system_lease(void);//是否租借

#endif
