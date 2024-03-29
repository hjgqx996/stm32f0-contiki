#ifndef __DEV_H__
#define __DEV_H__
#include "types.h"
#include "time.h"

void ld_dev_init(void);
/*===================================================
                系统运行时间ms
====================================================*/
extern void cpu_us_delay(int us);
extern void cpu_ms_delay(int ms);
#define delayus(n)   cpu_us_delay(n)  //us延时
#define delayms(n)   cpu_ms_delay(n)
void cpu_nvic_reset(void);

time_t time(time_t*t);
/*===================================================
                gpio
====================================================*/
extern void ld_gpio_init(void);
extern void ld_gpio_set(U32 index,U8 value);
extern U8 ld_gpio_get(U32 index);
extern U8 ld_gpio_refresh(void);
extern void ld_gpio_set_io(U32 index,BOOL out,U8 value);

extern void ld_hc595_init(void);
extern void ld_hc595_reload(void);
extern void ld_hc595_reload(void);
/*===================================================
                uart
====================================================*/
int ld_uart_init(void);
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


/*===================================================
                红外数据接口
====================================================*/
void ld_ir_timer_100us(void);
BOOL ld_ir_read_start(U8 ir,U8 re,U8 cmd,U8 wanlen);
int ld_ir_read_isok(U8*dataout,U8 size);
int ld_ir_read_state(void);                       /*查看红外当前故障类型:-1 ：前导码  -2:数据   0:NULL  2: 成功*/
/*===================================================
                iic数据接口
====================================================*/
BOOL ld_bq27541_read_words(U8 sda,U8 scl,U8*cmd,U8 cmdlen,U16 *dataout);
BOOL ld_bq27541_read_id(U8 sda,U8 scl,U8*dataout);
BOOL ld_bq27541_de_encrypt_charge(U8 sda,U8 scl,U8 cmd);
BOOL ld_bq27541_check_ack(U8 sda,U8 scl);
BOOL ld_bq27541_output_flag(U8 sda,U8 scl,U8*data);

/*===================================================
                flash读写数据接口
====================================================*/
void ld_flash_write(uint32_t WriteAddress, uint8_t *WriteBuf, int32_t WriteNum, uint32_t WriteAddr_page);
void ld_flash_read(uint32_t ReadAddress, uint8_t *ReadBuf, int32_t ReadNum,uint32_t WriteAddr_page);

/*===================================================
                外部中断
====================================================*/
void ld_exti_init(void);

/*===================================================
                定时器
====================================================*/

void ld_timer3_init(void);
/*===================================================
                看门狗
====================================================*/

void ld_iwdg_init(void);
void ld_iwdg_reload(void);

/*===================================================
                调试支持
====================================================*/
#pragma pack(1)
typedef struct {
	U16
	ch:8,     //仓道地址
	type:4,   //类型 1: 红外前导码 2:红外数据  3.读失败数据清0  4.摆臂开关无效清0 5.宝与仓道不符时id==0 6.宝与仓道不符时id!=0 7.宝与仓道不符pch=NULL
	          // 8 :无效通讯方式
	mode:2,   //当前通讯方式 0:无 1:iic 2:红外
	unused:2; //
	U8 cmd;   //当前指令
  U32 ms;//当前时间
}DebugInfo;
#pragma pack()
void ld_debug_printf(U8 type,U8 ch,U8 cmd,U8 mode);
void ld_debug_read(int o,char*d);
void ld_debug_printf_clear(void);
int ld_debug_counter(void);
#endif

