
#ifndef __25_50_NAS_H__
#define __25_50_NAS_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

//<s>PCB板编号
#define PCB_VERSION    "(LD-MZ-DRIVER-5-A-V2)"

//<o>硬件编号<0-0xFFFF:1>
//<i>带红外的硬件版本 0203
#define HARDWARE_VERSION          0x0201
//<o>软件编号<0-0xFFFF:1>
//<i>软件编号从0001开始叠加
#define SOFTWARE_VERSION          0x0018

//<o>仓道个数
#define CHANNEL_MAX               5

//<e>使用 3个HC595
#define USING_3_HC595             1
//</e>


//<o>串口波特率
#define BAUDRATE                  115200

//<e>不使用 IR
#define NOT_USING_IR               1
//</e>




//电磁阀电源使能
#define dian_ci_fa_power(enable)
//5V充电输出
#define set_out5v()     ld_gpio_set(pch->map->io_mp,1) //输出5V

//硬件io配置
#define driver_config_io_map() \
	/*电磁阀 摆臂开关 到位开关  红外发送  红外接收   led灯      充电使能  充电输出检测     sda       scl*/    \
	OUT(HC,2) IN(B,10) IN(B,15)  NoIO()    NoIO()    OUT(HC,0)  OUT(HC,1)  IN(C,0)         ODH(B,8)  ODH(B,9) \
	OUT(HC,6) IN(B,1)  IN(A,8)   NoIO()    NoIO()    OUT(HC,4)  OUT(HC,5)  IN(C,1)         ODH(B,6)  ODH(B,7) \
	OUT(HC,10)IN(A,0)  IN(F,6)   NoIO()    NoIO()    OUT(HC,8)  OUT(HC,9)  IN(C,5)         ODH(B,4)  ODH(B,5) \
	OUT(HC,14)IN(C,13) IN(F,7)   NoIO()    NoIO()    OUT(HC,12) OUT(HC,13) IN(C,4)         ODH(D,2)  ODH(B,3) \
	OUT(HC,18)IN(C,9)  IN(A,12)  NoIO()    NoIO()    OUT(HC,16) OUT(HC,17) IN(B,13)        ODH(C,11) ODH(C,12)

//硬件exti配置
#define driver_config_exti_map()\
	/*仓道1*/exti(B,15)  \
	/*仓道2*/exti(A, 8)  \
	/*仓道3*/exti(F, 6)  \
	/*仓道4*/exti(F, 7)  \
	/*仓道5*/exti(A,12)
	
	
	
#endif
