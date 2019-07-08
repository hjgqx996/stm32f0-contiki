


#include "stm32f0xx.h"
#include "types.h"
/*===================================================
               GPIO
====================================================*/
const static GPIO_TypeDef *GPIOS[]={(GPIO_TypeDef *)GPIOA,(GPIO_TypeDef *)GPIOB,(GPIO_TypeDef *)GPIOC,(GPIO_TypeDef *)GPIOD,(GPIO_TypeDef *)0,(GPIO_TypeDef *)GPIOF};
const static U32           GPIO_RCC[]={RCC_AHBPeriph_GPIOA,RCC_AHBPeriph_GPIOB,RCC_AHBPeriph_GPIOC,RCC_AHBPeriph_GPIOD,0,RCC_AHBPeriph_GPIOF};
void ld_gpio_init(GPIO_Map*map)
{
    if(map==NULL)return;
	{
	    GPIO_TypeDef * g = (GPIO_TypeDef *)GPIOS[map->port];
		U32 p = 1L<<(map->pin);
		if(map->port==4||map->port>=(sizeof(GPIOS)/sizeof(GPIO_TypeDef *)))return;
		{
			//³õÊ¼»¯Ê±ÖÓ
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_AHBPeriphClockCmd(GPIO_RCC[map->port], ENABLE);		
			
			GPIO_InitStructure.GPIO_Pin = p;
			GPIO_InitStructure.GPIO_Mode = (GPIOMode_TypeDef)map->mode;
			GPIO_InitStructure.GPIO_OType = (GPIOOType_TypeDef)map->otype;
			GPIO_InitStructure.GPIO_PuPd = (GPIOPuPd_TypeDef)map->pp;
			GPIO_InitStructure.GPIO_Speed = (GPIOSpeed_TypeDef)map->speed;
			GPIO_Init(g, &GPIO_InitStructure);	
		}

	}
}
