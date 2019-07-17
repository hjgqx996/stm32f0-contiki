#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"
#include "config.h"


/*ϵͳ���״̬*/
typedef enum{
	LEASE_STATE_INIT = 0, //��ʼ̬
	LEASE_STATE_LEASE,    //���̬
	LEASE_STATE_RETURN,   //�黹̬
	LEASE_STATE_CTRL,     //����̬

}LeaseState;



/*ϵͳ����*/
typedef struct{
	volatile U8 addr485;                 //485��ַ
	volatile U8 addr_ch[CHANNEL_MAX];    //ͨ����ַ
}System;

extern System system;

#endif
