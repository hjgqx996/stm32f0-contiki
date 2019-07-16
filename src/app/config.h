

#ifndef __CONFIG_H__
#define __CONFIG_H__

//数据包接收事件
#define PROCESS_EVENT_PACKET     0x40

//485串口号
#define COM_485                   2

//485发送使能
#define enable_485_tx()  ld_gpio_set(2,1)
#define disable_485_tx() ld_gpio_set(2,0)

#endif