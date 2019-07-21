

#include "includes.h"
#include "stm32f0xx_rcc.h"
/*===================================================
                ˽��
====================================================*/
#define APPLICATION_ADDRESS     ((U32)(BOOT_LOADER_SIZE +0x08000000))

void IAP_Set()
{
   uint32_t i = 0;
      
  for(i = 0; i < 48; i++)
  {
    *((uint32_t*)(0x20000000 + (i << 2)))=*(volatile U32*)(APPLICATION_ADDRESS + (i<<2));
	}
 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); /* Enable the SYSCFG peripheral clock*/ 
  SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);     /* Remap SRAM at 0x00000000 */
}	


void ld_iwdg_init(void)
{
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_64);//6.4MS
  IWDG_SetReload(3750);//3000       3125*64/40=5000ms
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}	
#define KR_KEY_RELOAD    ((uint16_t)0xAAAA)
void ld_iwdg_reload(void)
{
	IWDG->KR = KR_KEY_RELOAD;
}


/*===================================================
                �ײ��ʼ��
====================================================*/
void ld_dev_init(void)
{
	//bootloader���ض���������
	#if USING_BOOT_LOADER >1
	IAP_Set();
	#endif
	
	//gpio
	ld_gpio_init();
	
	//uart
	ld_uart_init();
	
	//���Ź�
	
	//�ⲿ�ж�
	ld_exti_init();

	//��ʱ��
	ld_timer3_init();
}
