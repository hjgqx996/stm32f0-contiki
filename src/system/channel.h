
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
	BAO_ALLOW_X = 0x07,  //??????
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
	
	/*--------------运行状态数据----------------------*/
	U8 iic_dir;                     //iic方向 0:正常方向  1:方向反转
	ChannelState state;             //运行状态
	ChannelWarn  warn;              //运行告警
	ChannelError error;             //运行错误
	
	S32 priority;                   //充电控制优先级-->充电输出使用(优先级+冒泡)排队充电
	
	FSM insert;                     //充电宝进入仓道状态机变量:私有
}Channel;

#pragma pack()

/*===================================================
                全局函数
====================================================*/
/*获取仓道数据
*channel:1-n
*/
Channel*channel_data_get(U8 channel);
#endif



