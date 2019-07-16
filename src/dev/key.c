/*========================================================
 
* file name :		key.c
* Author :		
* Version	��		v1.00
* Date	��		09/07/2011
* Description��	��������

*********************************************************
*Updata:  
*Author :		
*Data:			
*direction��  	
*********************************************************		
========================================================*/
#include "driver_config_types.h"
#include "cpu.h"
#include <stdlib.h>
#include <string.h>
/**************************************

	Configure  or  interface
***************************************/

extern const unsigned char 		key_number;       //io ����
extern t_key_map 						  key_map[]; //�˿����ò���
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
/* ��ʼ�������е�����io */
void ld_key_init(void)
{
//	int x = key_number;
//	while(x-->0)key_Config(x);
//�Ѿ���gpio��ʼ��
}

/**
* ��ѯ��ֵ,32λ�е�λ����1,��λ������ 
* values: ���µļ�ֵ������һ������,����Ϊ������,0 Ϊû�а���
* return: ���µļ�����Ӧ��λ
*/
U32 ld_key_read(U32*values)
{
	unsigned int i, n = 0;
	static unsigned int nSaved = 0;
	for (i = 0; i < key_number; i++) {
		//ʵ�ⶶ��200us
		if (key_Get(i) == keyDownLevel) 	
		{
			//cpu_us_delay(150);
			if (key_Get(i) == keyDownLevel)
				goto DOWN;
		}
		goto UP;
		DOWN:
		{
			//�а���������
			if (GETBIT(nSaved, i) == 0) {
				SETBIT(nSaved, i);
				SETBIT(n, i);
				values[i] = key_map[i].keyvalue;
			}
			//һֱ����
			continue;
		} 
		UP:
		//�а�������
		if (GETBIT(nSaved, i) == 1)
		{
			values[i] = key_map[i].keyvalue|0x80000000;
			SETBIT(n, i);
		} 
		CLRBIT(nSaved, i);
	}
	return n;
}

