

#ifndef __CONFIG_H__
#define __CONFIG_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

//数据包接收事件
#define PROCESS_EVENT_PACKET     0x40

//<o>485串口号  <1=>uart1 <2=>uart2
#define COM_485                   2

//485发送使能
#define enable_485_tx()   ld_gpio_set(2,1)
#define disable_485_tx()  ld_gpio_set(2,0)
#define enable_485_rx()   disable_485_tx()
#define disable_485_rx()  enable_485_tx()

//<o>仓道个数
#define CHANNEL_MAX                5

//<o>仓道编号长度
#define CHANNEL_ID_MAX             10

//<o>硬件编号<0x0-0xFFFF:4>
#define HARDWARE_VERSION          0x0202
//<o>软件编号<0x0-0xFFFF:4>
#define SOFTWARE_VERSION          0x0001



#endif