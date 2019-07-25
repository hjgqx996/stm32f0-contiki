#include "74HC595.h"

static void delay(uint32_t t)
{
//	uint32_t i;
//	while(t--)
//		for (i = 0; i < 1; i++);
	__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP(); 
	__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP(); 
	__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP(); 
	__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
}
//#define delay cpu_us_delay

void ld_hc595_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = HC595_CLK_PIN |HC595_CS_PIN|HC595_DATA_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed =GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	
	HC595_CLK_H();
	HC595_DATA_H();
	HC595_CS_H();	
}

void ld_hc595_reload(void)
{
	HC595_CS_L();
	delay(2);
	HC595_CS_H();
}

void ld_hc595_send(uint32_t data)
{
  uint8_t j;
  for (j = 0; j < 16; j++)
	{
    if((data<<j) & (0x8000))
		HC595_DATA_H();
		else
		HC595_DATA_L();
    HC595_CLK_L();              //上升沿发生移位		
		delay(1);
    HC595_CLK_H();
  }
	ld_hc595_reload();
}





  


