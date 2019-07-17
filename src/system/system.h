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



/*系统数据*/
typedef struct{
	volatile U8 addr485;                 //485地址
	volatile U8 addr_ch[CHANNEL_MAX];    //通道地址
}System;

extern System system;

#endif
