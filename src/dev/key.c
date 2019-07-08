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
extern const int key_maps_number;
extern t_key_maps key_maps[];

#define key_Config(map,X) 		    cpu_gpio_cfg(	map[(X)].gpio.xPort, \
																								map[(X)].gpio.xPin, \
																								map[(X)].gpio.xMode, \
																								map[(X)].gpio.xSta, \
																								map[(X)].gpio.xSpeed)
#define key_Get(map,X) 					  cpu_gpio_get(map[(X)].gpio.xPort,map[(X)].gpio.xPin)
#define key_Set(map,X,value) 			cpu_gpio_set(map[(X)].gpio.xPort,map[(X)].gpio.xPin,(value))


#define GETBIT(d,i)        (((d&(1<<i))==0)?(0):(1))
#define SETBIT(d,i)        d |= 1<<(i)
#define CLRBIT(d,i)        d &= ~(1<<(i))

/**************************************
	public
**************************************/
/* ��ʼ�������е�����io */
void gt_key_init(void)
{
	int i = 0;
	for(;i<key_maps_number;i++)
	{
		//��ʼ����
		int x = key_maps[i].number;
	  while(x-->0)key_Config(key_maps[i].map,x);
	}
}

/**
* ��ѯ��ֵ,32λ�е�λ����1,��λ������ 
* index : ��� 1-n
* values: ���µļ�ֵ������һ������,����Ϊ������,0 Ϊû�а���
* return: ���µļ�����Ӧ��λ
*/
extern int gt_runtime(void);
unsigned int gt_key_read(int index,unsigned char*values)
{
	unsigned int i, n[3] = {0,0,0};
	static unsigned int nSaved[3] = {0,0,0};
	t_key_map*map = key_maps[index-1].map;
	int key_number = key_maps[index-1].number;
	int downlevel = key_maps[index-1].keydown;
	if(index==0||index>key_maps_number)return 0;
	for (i = 0; i < key_number; i++) {
		//ʵ�ⶶ��200us
		if (key_Get(map,i) == downlevel) 	
				goto DOWN;
		goto UP;
		DOWN:
		{
			//�а���������
			if (GETBIT(nSaved[index-1], i) == 0) {
				SETBIT(nSaved[index-1], i);
				SETBIT(n[index-1], i);
				values[i] = map[i].keyvalue;
			}
			//һֱ����
			continue;
		} 
		UP:
		//�а�������
		if (GETBIT(nSaved[index-1], i) == 1)
		{
			values[i] = map[i].keyvalue|0x80;
			SETBIT(n[index-1], i);
		} 
		CLRBIT(nSaved[index-1], i);
	}
	return n[index-1];
}

