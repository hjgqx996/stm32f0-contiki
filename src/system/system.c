#include "includes.h"


System system;


/*系统数据初始化*/
void ld_system_init(void)
{
	//地址恢复 
	
}


/*系统闪灯://1: 100ms  2:500ms  3:2000ms*/
void ld_system_flash_led(U8 mode)
{
  system.led_flash_mode = mode;
	system.led_flash_time = 0;
}
/*系统灯定时器处理*/
void ld_system_led_timer(int ms)
{
	int per = 500;
	system.led_flash_time+=ms;
	switch(system.led_flash_mode)
	{
		case 1:per=500;break;
		case 2:per=100;break;
		case 3:per=200;break;
	}		
	if(system.led_flash_time>per)
	{
		system.led_flash_time=0;
		ld_gpio_set(1,!ld_gpio_get(1));
	}
}