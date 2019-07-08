#ifndef __TYPES_H__
#define __TYPES_H__
#include "stdint.h"


typedef uint8_t U8;
typedef int8_t S8;
typedef uint16_t U16;
typedef int16_t S16;
typedef uint32_t U32;
typedef int32_t S32;
typedef uint64_t U64;
typedef int64_t S64;

#ifndef NULL
#define NULL 0
#endif /* NULL */

//GPIO����������
typedef struct{
	U8 port:4;	                     //0-5 A-F
	U8 pin:4;                        //0-15 pin_0 -- pin_15
	U8 mode:2,otype:2,pp:2,speed:2;	 //ģʽ�������ʽ�������������ٶ�	��stm32������ͬ
}GPIO_Map;





#endif
