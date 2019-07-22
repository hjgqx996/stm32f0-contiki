

#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "contiki.h"
//===========================类型=============================//
typedef enum{
	PROCESS_EVENT_PACKET           =0x40,   //事件:数据包接收
	PROCESS_EVENT_COMM_LEASE,               //事件:租借数据包处理
	PROCESS_EVENT_COMM_CTRL,                //事件:控制命令
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
#define CHANNEL_MAX           5

//<o>允许同时充电的最大仓道数
#define CHANNEL_CHARGE_MAX    2

//<o>仓道编号长度
#define CHANNEL_ID_MAX             10

//<o>硬件编号<0x0-0xFFFF:4>
#define HARDWARE_VERSION          0x0202
//<o>软件编号<0x0-0xFFFF:4>
#define SOFTWARE_VERSION          0x0001

//<o>租借时，仓道灯闪烁频率ms
#define LEASE_LED_FLASH_TIME      200

//<o>仓道灯亮时，最小电量%
#define CHANNEL_LED_LIGHT_UFSOC   50

//<o>红外---->iic    切换时，通讯失败次数
#define IR_TO_IIC_SWITCH_ERROR_MAX   2

//<o>iic------>红外 切换时，通讯失败次数
#define IIC_TO_IR_SWITCH_ERROR_MAX   3

//<o>iic切换方向时，通讯失败次数
#define IIC_DIR_SWITCH_MAX        3

//<o>循环读充电宝时长(ms)
#define BAO_READ_DATA_MAX_MS      2500

//<o>告警温度(度)
#define BAO_WARN_TEMPERATURE      60
//<o>故障温度(度)
#define BAO_ERROR_TEMPERATURE      65
//<o>读失败尝试次数
#define BAO_READ_ERROR_RETYR_TIMES         6

//<e> 是否使用bootloader
#define USING_BOOT_LOADER                  0
//<o> bootloader大小<0x0-0xFFFF:4>
#define BOOT_LOADER_SIZE                   0x3000
//</e>

//<h>充电流程参数

//<h> 补充
//<o>停止补充电量%
#define BUCHONG_STOP_UFSOC_MAX            95
//<h>1小时补充
//<o>停止补充电量%
#define BUCHONG_1HOUR_STOP_UFSOC_MAX      85
//<o>补充次数
#define BUCHONG_1HOUR_TIMES               3
//</h>

//<o>无限补充时长 min
//<i>当电量<=85%
#define BUCHONG_CYCLE_TIMEOUT            180

//</h>

//<h>停止电流
//<o>停止电流大小(ma)
#define STOP_CURRENT_MAX                   100
//<o>持续时间(s)
#define STOP_CURRENT_TIMEOUT                120
//</h>

//</h>


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

/******************更新固件0x06**************************/
#define deal_fault	        0x00      
#define updata_model        0x01      
#define normal_model			  0x02      
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

