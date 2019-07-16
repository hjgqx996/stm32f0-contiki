#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"
#include "config.h"

typedef struct{
	volatile U8 addr485;                 //485��ַ
	volatile U8 addr_ch[CHANNEL_MAX];    //ͨ����ַ
}System;

extern System system;

#endif
