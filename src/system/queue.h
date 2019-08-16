#ifndef __QUEUE_H__
#define __QUEUE_H__
#include "types.h"

/*-----------------------------------------------------
* channel 充电断电挂起
-------------------------------------------------------*/
BOOL request_charge_on(U8 ch,U32 seconds,BOOL hard);/*申请充电*/
BOOL request_charge_off(U8 ch);/*中止充电*/
BOOL request_charge_hangup_all(U32 scondes);/*挂起所有输出*/
BOOL ld_is_queue_hang(void);/*充电调度器是否挂起?*/

BOOL request_charge_hangup(U8 ch);/*挂起一个充电*/
BOOL request_charge_recovery(U8 ch);/*恢复一个充电*/
#endif

