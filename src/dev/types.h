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
#ifndef time_t
#define time_t unsigned int
#endif

#ifndef NULL
#define NULL 0
#endif /* NULL */

#ifndef LOW
#define LOW 0
#endif /* LOW */

#ifndef HIGH
#define HIGH 1
#endif /* HIGH */

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
/* 状态机封装了switch-case(这也是contiki内核封装的思想)使得它可以:
* 1.自由定义自己的状态
* 2.只关注条件与跳转，不关心细节与框架的实现
* 3.状态机使用FSM私有变量，使得它可以保存状态机的数据(它是轻量级的),这样状态是 可重入 的
* 4.在可重入的基础上，加入了状态机延时,它使用 查询系统计时 的方式判断超时
* 5.延时的定义，巧妙使用了两个状态，并与状态定义结合，使得 延时函数 waitus waitms 可以
    使用在 for() while() 以及它们的 嵌套 循环之中
* note:
*   状态机(非阻塞)封装之后，可以像 面向过程编程(阻塞函数) 一样写法
*   好处是:  编写编程简单，方便维护，方便修改与调试
*/
/*状态机数据结构*/
typedef struct{
	U16   save;     //在状态机中使用for(...){waitms(...)},时必须保存当前状态,用于状态完成后恢复
  U16   line;    //状态，保存状态所在的行号
  char*	name;     //状态名，保存当前状态的名称
	volatile U32   end;      //延时结束时间
	//状态机内循环时，必须使用私有static变量for(i....)  for(j...)
	U8   i;         //相当于i
	U8   j;         //相当于j 
}FSM;

//定义开始状态：它是一个switch-case 0
#define Start(sname)             switch(fsm->line){case 0:fsm->name=#sname;
	
//定义一个状态: 它使用case __LINE__ 来定位
#define State(sname)   \
	/*延时前状态恢复*/             if(fsm->save)fsm->line=fsm->save;fsm->save=0; goto Exp; \
	/*标签,可以用goto来跳转到这里*/sname: fsm->end=0; fsm->name=#sname;fsm->line=__LINE__;fsm->save=0;goto Exp; \
	/*switch-case,重入时定位这里 */case __LINE__:
	
//定义一个Default状态
#define Default() default:Exp:{}}
	
//状态机内延时(非阻塞): 它使用两个 case 来实现(也就是使用两个匿名的状态来实现delay功能)
//now new 用来避免 __LINE__冲突
#define Waitx(us,now,new)		 \
	/*设置延时*/                   fsm->end=fsm_time+us; \
  /*保存当前状态*/               if(fsm->save==0)fsm->save=fsm->line; \
  /*跳到延时状态*/               fsm->line=__LINE__+now; goto Exp;case __LINE__+now: \
  /*判断是否超时*/               if((fsm_time-fsm->end)>0x80000000)goto Exp; \
  /*超时了*/                     fsm->line=__LINE__+new;fsm->end=0;case __LINE__+new:

/*延时ms*/
#define waitms(ms)               Waitx((ms*1000),10000,20000)
/*延时us>100us*/
#define waitus(us)               Waitx(us,10000,20000)
/*设置状态机当前时间*/
#define fsm_time_set(t)          fsm_time=t

/*===================================================
               os 延时
====================================================*/
#define os_delay(et,time) 		 etimer_set(&et, (time/10));PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et))
#endif
