/*========================================================

* file name 		:	drv_config.c
* Author 			:	kaihua.yang
* Version			��	v1.00
* Date				��	11/30/2015
* Description		��	Ӳ�����������ļ�

*********************************************************
*��Ӳ���йز����� Configure or interface����
*********************************************************
========================================================*/
/******************************************************************
			includes
*******************************************************************/
#include "driver_config_types.h"
#include "types.h"
//#include "gtDev.h"

/******************************************************************
			private
*******************************************************************/


/******************************************************************
			configure variables
*******************************************************************/
//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
/*****************
   �ⲿʱ������
*****************/
//<o> ʱ��--�ⲿ����


/*****************
   IDWG ��������
*****************/
//<o> iwdg
//const unsigned int iwatchdog_xpres =  IWATCHDOG_XPRES;
//const unsigned int iwatchdog_xsetr =  IWATCHDOG_XSETR;

#include "stm32f0xx.h"

/***********************
			GPIO  ���ó���
***********************/
#define PORT_A 0
#define PORT_B 1
#define PORT_C 2
#define PORT_D 3
#define PORT_E 4
#define PORT_F 5
#define PORT_HC  0xFF  
#define OUT(pp,pin)  {PORT_##pp,pin,GPIO_Mode_OUT,GPIO_OType_PP,GPIO_Speed_Level_3,0},
#define IN(pp,pin)   {PORT_##pp,pin,GPIO_Mode_IN,GPIO_OType_OD,GPIO_Speed_Level_3,0},

//<o> gpio
t_gpio_map gpio_map[]=
{
	/*ϵͳLED��*/
	OUT(F,5)
	
	/*��ŷ�*/
	OUT(C,2)   //M1_1N1
	OUT(C,3)   //M1_1N3
	OUT(B,14)  //M2_1N1
	OUT(B,11)  //M2_1N3
	OUT(C,6)   //M3_1N1
	
	/*�ֵ���*/
	OUT(HC,0)
	OUT(HC,4)
	OUT(HC,8)
	OUT(HC,12)
	OUT(HC,14)
	
	/*���ʹ��*/
	OUT(HC,1)
	OUT(HC,5)
	OUT(HC,9)
	OUT(HC,13)
	OUT(HC,15)
	
	/*���ⷢ��*/
	OUT(HC,2)
	OUT(HC,6)
	OUT(HC,7)
	OUT(HC,10)
	OUT(HC,11)	
	
	/*�������*/
  IN(C,14)
	IN(C,15)
	IN(F,0)
	IN(B,12)
	IN(C,7)
    // ��ʹ��
    0,
};
const unsigned char gpio_number = (sizeof(gpio_map)/sizeof(t_gpio_map)) - 1;

/***********************
			EXTI  ���ó���
***********************/
//<o> exti
t_exti_map exti_map[]={
 {0,15,0x08000,0x0C},
 0

};
const unsigned char exti_number = (sizeof(exti_map) /sizeof(t_exti_map)) - 1;

/*************************
	I2C ����  ģ��
*************************/

/*************************
			Uart
*************************/
