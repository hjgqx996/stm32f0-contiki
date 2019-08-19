
#ifndef __25_50_NAS_H__
#define __25_50_NAS_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//<e>用于25/50口机
#define APP_USING_25_50_MACHINE 1
//<s>PCB板编号
#define PCB_VERSION    "(LD-MZ-DRIVER-5-A-V6)"

//<o>硬件编号<0-0xFFFF:1>
//<i>带红外的硬件版本 0203
#define HARDWARE_VERSION          0x0203
//<o>软件编号<0-0xFFFF:1>
//<i>软件编号从0001开始叠加
#define SOFTWARE_VERSION          0x0011

//<o>仓道个数
#define CHANNEL_MAX           5

//</e>

//电磁阀电源使能
#define dian_ci_fa_power(enable)
//5V充电输出
#define set_out5v()     ld_gpio_set(pch->map->io_mp,1) //输出5V

#endif
