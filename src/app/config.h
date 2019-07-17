

#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "contiki.h"
//===========================类型=============================//
//读取充电宝-线程间通讯用
typedef struct{
	struct process* master;//请求线程
  
}ProcessData_iic_ir;

typedef enum{
	PROCESS_EVENT_PACKET           =0x40,   //事件:数据包接收
	PROCESS_EVENT_COMM_LEASE,               //事件:租借数据包处理
	PROCESS_EVENT_READ_ID,                  //事件:读充电ID
	PROCESS_EVENT_READ_DATA,                //事件:读数据
	PROCESS_EVENT_READ_UNLOCK,              //事件:解锁05
	PROCESS_EVENT_READ_LOCK,                //事件:上锁06
	PROCESS_EVENT_READ_UNLOCK_1HOUR,        //事件:解锁1小时07
	PROCESS_EVENT_READ_OUTPUT,              //事件:读输出标志
}PROCESS_EVENT;


//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
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

//<o>租借时，仓道灯闪烁频率ms
#define LEASE_LED_FLASH_TIME      200

//<o>仓道灯亮时，充电量最小电量%
#define CHANNEL_LED_LIGHT_UFSOC   50

//判断是否是6代宝
#define is_6_battery(ids)          if((ids[6]&0x0F)==0x06)              
//电磁阀输出
#define dian_ci_fa(channel,level)  ld_gpio_set(channel->map->io_motor,level)


/******************租借处理结果***************************/
#define Lease_fall   					0x00
#define Lease_success   			0x01
#define Lease_differ  				0x02
#define Lease_decrypt_fall    0x03
#define Lease_dianchifa_fall  0x04
/*********************************************************/

/******************归还处理结果***************************/
#define Return_fall   					0x00
#define Return_success   			  0x01
#define Return_unrecognized   	0x02
#define Return_timeout   	      0x03
/*********************************************************/

/******************控制命令结果***************************/
#define Cmd_fall   					  0x00
#define Cmd_success   			  0x01
/*********************************************************/

/******************红外命令*******************************/
#define IR_GETID          	10   //读取ID
#define IR_GETDATA          20   //获取数据
#define IR_OUTENABLE       	30   //允许输出
#define IR_OUTDISABLE      	50   //禁止输出
#define IR_OUTANHOUR      	40   //输出1小时



//==========================================================//
#include "channel.h"
extern ChannelConfigureMap channel_config_map[];




#endif