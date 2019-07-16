#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"
#include "config.h"

typedef struct{
	volatile U8 addr485;                 //485地址
	volatile U8 addr_ch[CHANNEL_MAX];    //通道地址
}System;

extern System system;

#endif
