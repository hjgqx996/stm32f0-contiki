#ifndef __QUEUE_H__
#define __QUEUE_H__
#include "types.h"

/*-----------------------------------------------------
* channel ���ϵ����
-------------------------------------------------------*/
BOOL request_charge_on(U8 ch,U32 seconds,BOOL hard);/*������*/
BOOL request_charge_off(U8 ch);/*��ֹ���*/
BOOL request_charge_hangup_all(U32 scondes);/*�����������*/
BOOL ld_is_queue_hang(void);/*���������Ƿ����?*/

BOOL request_charge_hangup(U8 ch);/*����һ�����*/
BOOL request_charge_recovery(U8 ch);/*�ָ�һ�����*/
#endif

