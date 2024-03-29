#include "74HC595.h"
#include "config.h"

static void delay()
{
	void cpu_us_delay(int us);
	#if USING_3_HC595 ==1
	cpu_us_delay(10);
	#else
	//cpu_us_delay(1);
	__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
	#endif
	
}
//extern void cpu_us_delay(int us);
//#define delay cpu_us_delay

void ld_hc595_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = HC595_CLK_PIN |HC595_CS_PIN|HC595_DATA_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
  GPIO_InitStruct.GPIO_Speed =GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	
	HC595_CLK_H();
	HC595_DATA_H();
	HC595_CS_H();	
}

void ld_hc595_reload(void)
{
	HC595_CS_L();
	delay();
	HC595_CS_H();
}

#if USING_3_HC595 ==0
void ld_hc595_send(uint32_t data)
{
  uint8_t j;
	uint8_t d = (data&0x8000)?1:0 ;
  for (j = 0; j < 16; j++)
	{
		//输出电平,延时一段时间
		if(d==1){
			HC595_DATA_H();
		}
		else{
			HC595_DATA_L();
		}
    HC595_CLK_L();              //上升沿发生移位		
		data<<=1;                   //延时
		d = (data&0x8000)?1:0;      //延时
		delay();
    HC595_CLK_H();              //拉高，可以改变电平
  }
	ld_hc595_reload();
}
#endif

#if USING_3_HC595 == 1
void ld_hc595_send(uint32_t data)
{
  uint8_t j;
	uint8_t d = (data&0x800000)?1:0 ;
  for (j = 0; j < 24; j++)
	{
		d=(d==1)?(HC595_DATA_H()):( HC595_DATA_L());//输出电平,延时一段时间
    HC595_CLK_L();              //上升沿发生移位		
		data<<=1;                   //延时
		d = (data&0x800000)?1:0;      //延时
		delay();
    HC595_CLK_H();              //拉高，可以改变电平
  }
	ld_hc595_reload();
}
#endif





  


