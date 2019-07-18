
#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include "types.h"
/*===================================================
                宏，类型
====================================================*/
#pragma pack(1)

/*仓道接口配置*/
typedef const struct{
	U8 io_motor;       //电磁阀io口
	U8 io_sw;          //摆臂开关io口
  U8 io_detect;      //到位开关io口
	
  U8 io_ir;          //红外发送io口
  U8 io_re;          //红外接收io口
	
  U8 io_led;         //led灯io口
  U8 io_mp;          //充电使能io口
  U8 io_mp_detect;   //充电输出检测io口
	
	U8 io_sda;         //iic sda口
	U8 io_scl;         //iic scl口
}ChannelConfigureMap;


/*仓道状态数据结构:低位在前*/
typedef struct{
	U8 \
	read_error:1,    //有宝，但是读取错误
	read_ok:1,       //有宝，但是读取正确
  reversed:3,      //保留
	read_from_ir:1,  //使用红外读取
	full_charge:1,   //充电满
	charging:1;      //有宝，数据读取正常，正在充电
}ChannelState;

/*仓道警告数据结构:低位在前*/
typedef struct{
	U8 \
	temperature:1,   //温度报警 <0 || >60
	eject:1,         //异常弹仓事件，本标志20分钟后清0
  mp:1,            //5V充电输出报警(有输出但检测不到输出 或 无输出但检测到输出)
	reversed:1,      //保留
	restart:1,       //模组异常重启报警
	reversed2:3;      //保留
}ChannelWarn;

/*仓道错误数据结构:低位在前*/
typedef struct{
	U8 \
	lease:1,         //租借故障
	motor:1,         //电磁阀/电机故障
  sw:1,            //摆臂开关故障
	ir:1,            //红外识别故障
	key:1,           //到位开关故障
	bao:1,           //来电宝故障  temp<0 || temp>65
	thimble:1,       //顶针识别故障
	reversed:1;      //保留
}ChannelError;


/*充电宝输出标志*/
typedef enum{
	BAO_ALLOW   = 0x05,  //充电宝允许输出
	BAO_NOTALLOW= 0x06,  //充电宝不允许输出
	BAO_ALLOW_ONE_HOUR = 0x07,  //允许输出1小时
}BaoOutput;

/*仓道数据结构*/
typedef struct{
	/*-------------通道数据--------------------------*/
	volatile U8  Ufsoc;             //剩余电量  %
	volatile U16 Voltage;           //电压,单位 mV
	volatile U16 RemainingCapacity; //剩余容量 mAh
	volatile U16 CycleCount;        //充电循环次数
	volatile S16 AverageCurrent;    //充电平均电流  mA
  volatile S8 Temperature;        //电芯温度	    度
	volatile U8 Ver;                //红外充电宝版本
	
	U8 id[10];                      //充电宝编号
	U8 addr;                        //通道地址flash[]<----System.addr_ch-------Channel.addr
	BaoOutput bao_output;           //充电宝输出标志 
	
	/*--------------配置接口--------------------------*/
	ChannelConfigureMap*map;        //通道控制io配置
	
	/*--------------iic方向切换------------------------*/
	U8  iic_dir;                     //iic方向 0:正常方向  1:方向反转
	U8  iic_dir_counter;             //出错计数
	
	/*--------------iic,ir切换------------------------*/
	U8  iic_ir_mode;                 //iic方向 0:正常方向  1:方向反转
	int iic_ir_mode_counter;         //出错计数
	
	/*--------------运行状态数据----------------------*/
	ChannelState state;             //运行状态
	ChannelWarn  warn;              //运行告警
	ChannelError error;             //运行错误
	
	/*--------------异常弹仓--------------------------*/
	
	/*--------------仓道灯----------------------------*/
	BOOL flash;                     //是否闪烁
	int  flash_ms;                  //闪烁总时间(ms)
	int  flash_now;                 //计时
	
	FSM insert;                     //充电宝进入仓道状态机变量:私有
}Channel;

#pragma pack()

/*===================================================
                 充电宝统一红外iic读类型
====================================================*/
/*  命令 */
typedef enum{
	RC_READ_ID   =10,      //:读充电ID
	RC_READ_DATA =20,      //:读数据
	RC_UNLOCK    =30,      //:解锁05
	RC_LOCK      =50,      //:上锁06
	RC_UNLOCK_1HOUR =40,   //:解锁1小时07
	RC_OUTPUT,             //:读输出标志
}READ_TYPE_CMD;          //充电宝命令
/*  通讯方式 */
typedef enum{
	RTM_IIC=0,
	RTM_IR=1,
}READ_TYPE_MODE;

/*===================================================
                全局函数
====================================================*/
BOOL channel_id_is_not_null(U8*id);
/*
* channel数据初始化
*/
BOOL channel_data_init(void);
BOOL channel_data_clear_by_addr(U8 ch_addr);//清数据

/*获取仓道数据
*channel:1-n
*/
Channel*channel_data_get(U8 channel);

/*获取仓道数据--by addr
*channel:1-n
*/
Channel*channel_data_get_by_addr(U8 addr);


/*通道灯闪烁控制*/
void channel_led_flash(U8 ch,U8 seconds);
void channels_les_flash_timer(int timer_ms);

/*----------------------------------
充电宝操作重定向:是否忙,读 ,是否完成
-----------------------------------*/
BOOL channel_read_busy(U8 ch,READ_TYPE_MODE mode);
BOOL channel_read_start(U8 ch,READ_TYPE_MODE mode,BOOL opposite,READ_TYPE_CMD cmd);
int channel_read_end(U8 ch,READ_TYPE_MODE mode,U8*dataout);

#endif



