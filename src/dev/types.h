#ifndef __TYPES_H__
#define __TYPES_H__
#include "stdint.h"

/*常用数据类型*/
typedef uint8_t U8;
typedef int8_t S8;
typedef uint16_t U16;
typedef int16_t S16;
typedef uint32_t U32;
typedef int32_t S32;
typedef uint64_t U64;
typedef int64_t S64;
typedef enum{ TRUE=1,FALSE=!TRUE}  BOOL;

#ifndef NULL
#define NULL 0
#endif /* NULL */
/*===================================================
                配置类型
====================================================*/
//GPIO的配置类型
typedef struct{
	U8 port:4;	                     //0-5 A-F
	U8 pin:4;                        //0-15 pin_0 -- pin_15
	U8 mode:2,otype:2,pp:2,speed:2;	 //模式，输出方式，上拉下拉，速度	与stm32定义相同
}GPIO_Map;


/*===================================================
                状态机定义
====================================================*/
typedef struct{
  U32   state;    //状态，保存状态所在的行号
  void*	name;     //状态名，保存当前状态的名称
}FSM;
#define Start(sname)                switch(fsm->state){case 0:fsm->name=#sname;
#define State(sname)                break;sname: fsm->state=__LINE__;break;case __LINE__:fsm->name=#sname;

#endif
