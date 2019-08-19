
#ifndef __8_16_NAS_H__
#define __8_16_NAS_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//<e>用于8/16口机
#define APP_USING_8_16_MACHINE 1
//<s>PCB板编号
#define PCB_VERSION    "(LD-MZ-DRIVER-8-A-V21.0)"

//<o>硬件编号<0-0xFFFF:1>
//<i>带红外的硬件版本 0203
#define HARDWARE_VERSION          0x0203
//<o>软件编号<0-0xFFFF:1>
//<i>软件编号从0001开始叠加
#define SOFTWARE_VERSION          0x0006

//<o>仓道个数
#define CHANNEL_MAX           4

//</e>

//电磁阀电源使能
#define dian_ci_fa_power(enable)   ld_gpio_set(43,enable)

//5V充电输出
#define set_out5v()      do{ld_gpio_set_io(pch->map->io_mp_detect,TRUE,1);ld_gpio_set(pch->map->io_mp,1);ld_gpio_set_io(pch->map->io_mp_detect,FALSE,0);}while(0)

#endif
