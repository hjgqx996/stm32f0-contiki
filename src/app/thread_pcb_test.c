
#include "includes.h"

HPacket hpacket;
BOOL is_system_lease(void){return FALSE;}
void channel_lock_timer(int ms){}
int channel_read_delay_ms;
void charge_fms_timer(int ms){}
void ld_system_led_timer(int ms){}
	
/*自动运行线程地址*/
extern const  U32 autostart$$Base;
extern const  U32 autostart$$Limit;	
int main(void)
{
	channel_data_init();//初始化仓道数据
	ld_system_init();   //系统参数  
	ld_dev_init();      //底层初始化
	clock_init();       //系统调度时钟
	process_init();     //线程初始化
	process_start(&etimer_process,NULL); //启动定时器
	autostart_start((struct process * const*)&autostart$$Base,((int)((int)&autostart$$Limit-(int)&autostart$$Base))/4);//自动运行线程
	
	while(1){
		while(process_run()> 0);//线程循环  
    ld_iwdg_reload();		
	}
}

///*===================================================
//               本地函数
//====================================================*/
static void all_output(BOOL level)
{
	int i=0;
	Channel*pch=NULL;
	for(i=1;i<=CHANNEL_MAX;i++)
	{
		/* 1 秒钟 开所有输出*/
		pch = channel_data_get(i);
		dian_ci_fa_power(level==TRUE?1:0);                  //电磁阀电源
		dian_ci_fa(pch,level==TRUE?HIGH:LOW);               //电磁阀
		ld_gpio_set(pch->map->io_led,level==TRUE?HIGH:LOW); //LED灯
		if(level==TRUE)                                     //充电输出
		{
			set_out5v();   
		}else
		{
		 reset_out5v();
		}			
		ld_gpio_set(1,level==TRUE?0:1);                     //系统灯
	}
	ld_gpio_refresh();
}
///*===================================================
//                全局函数
//====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(pcb_test)
{
	ld_iwdg_reload();             //喂狗
	PROCESS_BEGIN();
	while(1)
	{
			all_output(TRUE);	
			ld_iwdg_reload();                   //喂狗
			os_delay(pcb_test,1000);
		
		  all_output(FALSE);	
			ld_iwdg_reload();                   //喂狗
			os_delay(pcb_test,1000);			
		  channel_data_map_init();        //2019-9-19: 端口配置初始化:长时间运行时不排除加载到内存中的端口配置出现问题,引起所有通道不正常工作
	}
	PROCESS_END();
}


/**
  ******************************************************************************
  * @file    stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    02-October-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "types.h"
/** @addtogroup STM32F0308-Discovery_Demo
  * @{
  */

/** @addtogroup STM32F0XX_IT
  * @brief Interrupts driver modules
  * @{
  */
void NMI_Handler(void){}
void HardFault_Handler(void){}
void SVC_Handler(void){}
void PendSV_Handler(void){}

#if NOT_USING_IR ==0
void TIM3_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) //溢出中断
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位  
	}
}
#endif



/**
  * @brief  This function handles USART1_IRQHandler.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
	USART_TypeDef *pUart=USART1;
	char temp;
  /* USART in mode Receiver --------------------------------------------------*/
  if(USART_GetITStatus(USART1, USART_IT_RXNE)  != RESET)
  {
		USART_ClearITPendingBit(pUart, USART_IT_RXNE);						
  }
	
	if(USART_GetITStatus(USART1,USART_IT_TXE)==SET)
	{
		USART_ITConfig(pUart, USART_IT_TXE, DISABLE);
	}	

	if(USART_GetFlagStatus(pUart,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_ORE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_NE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_NE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_FE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_FE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}
	
	if(USART_GetFlagStatus(pUart,USART_FLAG_PE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_PE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}	 			
	
}

void USART2_IRQHandler(void)
{
	USART_TypeDef *pUart=USART2;
  /* USART in mode Receiver --------------------------------------------------*/
  if(USART_GetITStatus(pUart, USART_IT_RXNE)  != RESET)
  {
		USART_ClearITPendingBit(pUart, USART_IT_RXNE);						
  }
	
	if(USART_GetITStatus(pUart,USART_IT_TXE)==SET)
	{

		USART_ITConfig(pUart, USART_IT_TXE, DISABLE);
	}	
	
	if(USART_GetFlagStatus(pUart,USART_FLAG_TC)==SET)
	{
		enable_485_rx();
		USART_ClearFlag(pUart,USART_FLAG_TC);	//读SR
	}
	
	if(USART_GetFlagStatus(pUart,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_ORE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_NE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_NE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_FE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_FE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}
	
	if(USART_GetFlagStatus(pUart,USART_FLAG_PE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_PE);	//读SR
		USART_ReceiveData(pUart);	//读DR
	}	 			
} 


/*
* 开关中断，触发充电状态机
*/
void EXTI4_15_IRQHandler(void)
{ 
    /* Clear the EXTI line 6 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line15|EXTI_Line12|EXTI_Line8|EXTI_Line7|EXTI_Line6);
}

