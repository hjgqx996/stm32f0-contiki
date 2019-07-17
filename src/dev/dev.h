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
	打开串口
	
	xUart  :1-n  Uart1-Uartn
	xBaud  :波特率
	xData  :数据位，5 6 7 8 
	xParity:无 0，偶 1，奇2
	xStop  :停止位 0:0.5bit   1:1bit  2:1.5bit  3:2bit
	rxbufsize :接收缓存  >0 以中断方式接收
	txbufsize :发送缓存  >0 以中断方式发送

	return :  TRUE 成功  FALSE 失败
*/
int ld_uart_open(U8 xUart,int xBaud,U8 xData,U8 xParity, U8 xStop,int rxbufsize,int txbufsize);
int ld_uart_close(U8 xUart);
int ld_uart_send(U8 xUart,U8*pBuf,int size);
int ld_uart_read(U8 xUart,U8*pBuf,int size);
int ld_uart_dump(U8 xUart,U8 type);//清收发缓冲:type  :bit0 :rx data    bit1 tx data
int ld_uart_isp(U8 xUart,char*byte,U8 type);	//type: 0 接收    1发送




/*红外操作接口
* ch    :仓道号 1-n
* io_ir :发送端口
* io_re :接收端口
*/
void ld_ir_init(U8 ch,U8 io_ir,U8 io_re);
void ld_ir_timer_init(void);
void ld_ir_timer_100us(void);
BOOL ld_ir_read_start(U8 ch,U8 cmd,U8 wanlen);
int ld_ir_read_isok(U8 ch,U8*dataout,U8 size);

#endif
