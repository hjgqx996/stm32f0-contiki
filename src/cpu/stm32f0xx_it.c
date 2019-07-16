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
#include "includes.h"
/** @addtogroup STM32F0308-Discovery_Demo
  * @{
  */

/** @addtogroup STM32F0XX_IT
  * @brief Interrupts driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles TIM3 global interrupt request.
  * @param  None
  * @retval None
  */
extern void ld_ir_timer_100us(void);
void TIM3_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) //溢出中断
	{
			ld_ir_timer_100us();
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位  
	}
	
}


/**
  * @brief  This function handles USART1_IRQHandler.
  * @param  None
  * @retval None
  */

PROCESS_NAME(thread_comm);
extern HPacket hpacket;
void USART1_IRQHandler(void)
{
	USART_TypeDef *pUart=USART1;
	char temp;
  /* USART in mode Receiver --------------------------------------------------*/
  if(USART_GetITStatus(USART1, USART_IT_RXNE)  != RESET)
  {
		temp = USART_ReceiveData(pUart)& 0xff;
		ld_uart_isp(1,&temp,0);
		USART_ClearITPendingBit(pUart, USART_IT_RXNE);						
  }
	
	if(USART_GetITStatus(USART1,USART_IT_TXE)==SET)
	{
		if(ld_uart_isp(1,&temp,1) != FALSE)
		{
				USART_SendData(pUart, temp);
		}
		else
		{
			 // pUart->CR1 &= ~(USART_FLAG_TXE | USART_FLAG_TC);
				USART_ITConfig(pUart, USART_IT_TXE, DISABLE);
				//USART_ITConfig(pUart, USART_IT_TC, ENABLE);
		}
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
/**
  * @brief  This function handles USART2_IRQHandler.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
	USART_TypeDef *pUart=USART2;
	char temp;
  /* USART in mode Receiver --------------------------------------------------*/
  if(USART_GetITStatus(pUart, USART_IT_RXNE)  != RESET)
  {
		temp = USART_ReceiveData(pUart)& 0xff;
	  if(packet_recv(temp,&hpacket)!=NULL)
		{
			//发送一个事件给Comm任务
			process_post(&thread_comm,PROCESS_EVENT_PACKET,(void*)&hpacket.p);
			//关接收
			disable_485_rx();
		}
		//ld_uart_isp(2,&temp,0);
		USART_ClearITPendingBit(pUart, USART_IT_RXNE);						
  }
	
	if(USART_GetITStatus(pUart,USART_IT_TXE)==SET)
	{

			if(ld_uart_isp(2,&temp,1) != FALSE)
			{
					USART_SendData(pUart, temp);
			}
			else
			{
				 //使能接收
				 enable_485_rx();
				 // pUart->CR1 &= ~(USART_FLAG_TXE | USART_FLAG_TC);
					USART_ITConfig(pUart, USART_IT_TXE, DISABLE);
					//USART_ITConfig(pUart, USART_IT_TC, ENABLE);
			}
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

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0x8.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
