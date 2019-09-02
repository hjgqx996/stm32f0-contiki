#include "includes.h"

//唯一全局变量，到处飞
System system;
/*===================================================
                全局函数
====================================================*/

/*系统数据初始化*/
void ld_system_init(void)
{
	//地址恢复 --从flash中读取
	ld_flash_read(0, (U8*)&system.addr485, 6,RS485_ADDR);//读取485地址
	
	//是否支持IR,不支持IR时，只使用IIC
	#ifdef NOT_USING_IR
	system.iic_ir_mode = SIIM_ONLY_IIC; 
	#else
	system.iic_ir_mode = SIIM_IIC_IR;                    //默认iic ir都支持
	#endif
	
	channel_addr_set((U8*)system.addr_ch);               //将地址设置到Channel变量看    

  //充电使能:默认:冒泡排队，使能输出
  system.mode = 0;
	system.enable = TRUE;
	memset(system.chs,0,CHANNEL_MAX);
}


/*系统闪灯:mode: 100ms  500ms 2000ms*/
void ld_system_flash_led(int mode,int seconds)
{
  system.led_flash_mode = mode;
	system.led_flash_total_time = seconds*1000;
}


/*系统灯定时器处理*/
void ld_system_led_timer(int ms)
{
	system.led_flash_time+=ms;	
  system.led_flash_total_time-=ms;	
	if(system.led_flash_time>system.led_flash_mode)
	{
		system.led_flash_time=0;
		ld_gpio_set(1,!ld_gpio_get(1));
	}
	//闪烁时间到变为 2000 秒闪烁
	if(system.led_flash_total_time<=0)
	{
		system.led_flash_mode = 2000;
		system.led_flash_total_time=3600000;
	}
}


