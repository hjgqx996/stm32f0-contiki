

#include "stm32f0xx.h"
#include "types.h"



void ld_dev_init(void)
{

	//bootloader下重定向向量表
	
	//gpio
	ld_gpio_init();

}
