#ifndef __DEV_H__
#define __DEV_H__
#include "types.h"


/*===================================================
                gpio
====================================================*/
extern void ld_gpio_init(void);
extern void ld_gpio_set(U32 index,U8 value);
extern U8 ld_gpio_get(U32 index);

/*===================================================
                uart
====================================================*/
int ld_uart_init();
/*
	�򿪴���
	
	xUart  :1-n  Uart1-Uartn
	xBaud  :������
	xData  :����λ��5 6 7 8 
	xParity:�� 0��ż 1����2
	xStop  :ֹͣλ 0:0.5bit   1:1bit  2:1.5bit  3:2bit
	rxbufsize :���ջ���  >0 ���жϷ�ʽ����
	txbufsize :���ͻ���  >0 ���жϷ�ʽ����

	return :  TRUE �ɹ�  FALSE ʧ��
*/
int ld_uart_open(U8 xUart,int xBaud,U8 xData,U8 xParity, U8 xStop,int rxbufsize,int txbufsize);
int ld_uart_close(U8 xUart);
int ld_uart_send(U8 xUart,U8*pBuf,int size);
int ld_uart_read(U8 xUart,U8*pBuf,int size);
int ld_uart_dump(U8 xUart,U8 type);//���շ�����:type  :bit0 :rx data    bit1 tx data
int ld_uart_isp(U8 xUart,char*byte,U8 type);	//type: 0 ����    1����



#endif
