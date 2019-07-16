/*========================================================
 
* file name :		key.c
* Author :		
* Version	：		v1.00
* Date	：		09/07/2011
* Description：	按键驱动

*********************************************************
*Updata:  
*Author :		
*Data:			
*direction：  	
*********************************************************		
========================================================*/
#include "driver_config_types.h"
#include "cpu.h"
#include <stdlib.h>
#include <string.h>
/**************************************

	Configure  or  interface
***************************************/

extern const unsigned char 		key_number;       //io 个数
extern t_key_map 						  key_map[]; //端口配置参数
extern const unsigned char    keyDownLevel;

#define key_Config(X) 		    cpu_gpio_cfg(key_map[(X)].gpio.xPort, \
															key_map[(X)].gpio.xPin, \
															key_map[(X)].gpio.xMode, \
															key_map[(X)].gpio.xSta, \
															key_map[(X)].gpio.xSpeed)
#define key_Get(X) 					  cpu_gpio_get(key_map[(X)].gpio.xPort,key_map[(X)].gpio.xPin)
#define key_Set(X,value) 			cpu_gpio_set(key_map[(X)].gpio.xPort,key_map[(X)].gpio.xPin,(value))


#define GETBIT(d,i)        (((d&(1<<i))==0)?(0):(1))
#define SETBIT(d,i)        d |= 1<<(i)
#define CLRBIT(d,i)        d &= ~(1<<(i))

/**************************************
	public
**************************************/
/* 初始化配置中的所有io */
void ld_key_init(void)
{
//	int x = key_number;
//	while(x-->0)key_Config(x);
//已经在gpio初始化
}

/**
* 查询键值,32位中的位被置1,该位被按下 
* values: 按下的键值，它是一个数组,长度为键个数,0 为没有按下
* return: 按下的键，对应的位
*/
U32 ld_key_read(U32*values)
{
	unsigned int i, n = 0;
	static unsigned int nSaved = 0;
	for (i = 0; i < key_number; i++) {
		//实测抖动200us
		if (key_Get(i) == keyDownLevel) 	
		{
			//cpu_us_delay(150);
			if (key_Get(i) == keyDownLevel)
				goto DOWN;
		}
		goto UP;
		DOWN:
		{
			//有按键被按下
			if (GETBIT(nSaved, i) == 0) {
				SETBIT(nSaved, i);
				SETBIT(n, i);
				values[i] = key_map[i].keyvalue;
			}
			//一直按下
			continue;
		} 
		UP:
		//有按键弹起
		if (GETBIT(nSaved, i) == 1)
		{
			values[i] = key_map[i].keyvalue|0x80000000;
			SETBIT(n, i);
		} 
		CLRBIT(nSaved, i);
	}
	return n;
}

