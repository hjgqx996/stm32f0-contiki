
#include "includes.h"

HPacket hpacket;
BOOL is_system_lease(void){return FALSE;}
void channel_lock_timer(int ms){}
int channel_read_delay_ms;
void charge_fms_timer(int ms){}
void ld_system_led_timer(int ms){}
	
/*�Զ������̵߳�ַ*/
extern const  U32 autostart$$Base;
extern const  U32 autostart$$Limit;	
int main(void)
{
	channel_data_init();//��ʼ���ֵ�����
	ld_system_init();   //ϵͳ����  
	ld_dev_init();      //�ײ��ʼ��
	clock_init();       //ϵͳ����ʱ��
	process_init();     //�̳߳�ʼ��
	process_start(&etimer_process,NULL); //������ʱ��
	autostart_start((struct process * const*)&autostart$$Base,((int)((int)&autostart$$Limit-(int)&autostart$$Base))/4);//�Զ������߳�
	
	while(1){
		while(process_run()> 0);//�߳�ѭ��  
    ld_iwdg_reload();		
	}
}

///*===================================================
//               ���غ���
//====================================================*/
static void all_output(BOOL level)
{
	int i=0;
	Channel*pch=NULL;
	for(i=1;i<=CHANNEL_MAX;i++)
	{
		/* 1 ���� ���������*/
		pch = channel_data_get(i);
		dian_ci_fa_power(level==TRUE?1:0);                  //��ŷ���Դ
		dian_ci_fa(pch,level==TRUE?HIGH:LOW);               //��ŷ�
		ld_gpio_set(pch->map->io_led,level==TRUE?HIGH:LOW); //LED��
		if(level==TRUE)                                     //������
		{
			set_out5v();   
		}else
		{
		 reset_out5v();
		}			
		ld_gpio_set(1,level==TRUE?0:1);                     //ϵͳ��
	}
	ld_gpio_refresh();
}
///*===================================================
//                ȫ�ֺ���
//====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(pcb_test)
{
	ld_iwdg_reload();             //ι��
	PROCESS_BEGIN();
	while(1)
	{
			all_output(TRUE);	
			ld_iwdg_reload();                   //ι��
			os_delay(pcb_test,1000);
		
		  all_output(FALSE);	
			ld_iwdg_reload();                   //ι��
			os_delay(pcb_test,1000);			
		  channel_data_map_init();        //2019-9-19: �˿����ó�ʼ��:��ʱ������ʱ���ų����ص��ڴ��еĶ˿����ó�������,��������ͨ������������
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
  if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) //����ж�
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ  
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
		USART_ClearFlag(pUart,USART_FLAG_ORE);	//��SR
		USART_ReceiveData(pUart);	//��DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_NE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_NE);	//��SR
		USART_ReceiveData(pUart);	//��DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_FE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_FE);	//��SR
		USART_ReceiveData(pUart);	//��DR
	}
	
	if(USART_GetFlagStatus(pUart,USART_FLAG_PE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_PE);	//��SR
		USART_ReceiveData(pUart);	//��DR
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
		USART_ClearFlag(pUart,USART_FLAG_TC);	//��SR
	}
	
	if(USART_GetFlagStatus(pUart,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_ORE);	//��SR
		USART_ReceiveData(pUart);	//��DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_NE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_NE);	//��SR
		USART_ReceiveData(pUart);	//��DR
	}

	if(USART_GetFlagStatus(pUart,USART_FLAG_FE)==SET)
	{
		USART_ClearFlag(USART2,USART_FLAG_FE);	//��SR
		USART_ReceiveData(pUart);	//��DR
	}
	
	if(USART_GetFlagStatus(pUart,USART_FLAG_PE)==SET)
	{
		USART_ClearFlag(pUart,USART_FLAG_PE);	//��SR
		USART_ReceiveData(pUart);	//��DR
	}	 			
} 


/*
* �����жϣ��������״̬��
*/
void EXTI4_15_IRQHandler(void)
{ 
    /* Clear the EXTI line 6 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line15|EXTI_Line12|EXTI_Line8|EXTI_Line7|EXTI_Line6);
}

