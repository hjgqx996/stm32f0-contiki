
/*========================================================

* file name 		:	drv_config.h
* Author 			:	kaihua.yang
* Version			：	v1.00
* Date				：	11/30/2015
* Description		：	硬件驱动配置文件

*********************************************************
*与硬件有关部分在 Configure or interface给出
*********************************************************
========================================================*/


#ifndef __DRIVER_CONFIG__
#define __DRIVER_CONFIG__

#include "types.h"


//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

/******************************************
		时钟
*******************************************/
//<h> 时钟配置


//<e> 外部晶振
//<i> 如果不使用外部晶振，就使用内部8MHz HSI
#define HSE_ENABLE    0
//<o> 外部晶振频率
//  <i> Default:8000000
//	<i> 系统时钟 = 晶振 * PLL_MUL
#define HSE_DEFAULT_VALUE 8000000


//<h> 系统时钟 = 晶振 * 倍频 mul
//<o> MUL
//  <i> 范围 2- 16
//  <i> Defalult : 9  
#define HSE_PLL_MUL   9

//<o> 系统分频 
//<i> 只能是 1,2,4,8,16,64,128,256,512
//<i> default:1
#define HSE_SYSCLK_DIV 1
//</h>

//<h> AHB1 (max = 42M)
// <o> AHB1 分频  <1=>RCC_HCLK_Div1 <2=>RCC_HCLK_Div2 <4=>RCC_HCLK_Div4 <8=>RCC_HCLK_Div8 <16=>RCC_HCLK_Div16
// <i> AHB1 频率 = 系统时钟 / 分频
// <i> Default:36 Mhz
#define HSE_AHB1_DIV	2
//</h>

//<h> AHB2 (max = 72M)
// <o> AHB2 分频  <1=>RCC_HCLK_Div1 <2=>RCC_HCLK_Div2 <4=>RCC_HCLK_Div4 <8=>RCC_HCLK_Div8 <16=>RCC_HCLK_Div16
// <i> AHB2 频率 = 系统时钟 / 分频
// <i> Default: 72Mhz
#define HSE_AHB2_DIV	1
//</h>

//</e>


//<e> 内部时钟
//<i> HSI时钟为8MHz
#define HSI_ENABLE    1
//<o> 外部晶振频率
//  <i> Default:8000000
//	<i> 系统时钟 = 晶振 * PLL_MUL / DIV
#define HSI_DEFAULT_VALUE 8000000


//<h> 系统时钟 = 晶振 * 倍频 mul  /2/分频
//<o> MUL
//  <i> 范围 2- 16
//  <i> Defalult : 9  
#define HSI_PLL_MUL   9

//<o> 系统分频 
//<i> 只能是 1,2,4,8,16,64,128,256,512
//<i> default:1
#define HSI_SYSCLK_DIV 1
//</h>

//<h> AHB1 (max = 42M)
// <o> AHB1 分频  <1=>RCC_HCLK_Div1 <2=>RCC_HCLK_Div2 <4=>RCC_HCLK_Div4 <8=>RCC_HCLK_Div8 <16=>RCC_HCLK_Div16
// <i> AHB1 频率 = 系统时钟 / 分频
// <i> Default:36 Mhz
#define HSI_AHB1_DIV	1
//</h>

//<h> AHB2 (max = 72M)
// <o> AHB2 分频  <1=>RCC_HCLK_Div1 <2=>RCC_HCLK_Div2 <4=>RCC_HCLK_Div4 <8=>RCC_HCLK_Div8 <16=>RCC_HCLK_Div16
// <i> AHB2 频率 = 系统时钟 / 分频
// <i> Default: 72Mhz
#define HSI_AHB2_DIV	1
//</h>
//</e>

//</h>


/******************************************
		IWDG
*******************************************/
//<e>看门狗
//<i> 频率 = 主频(32KHz) / (sertr) /pres
#define IWDG_INTERFACE_ENABLE			  0
//==============================
//<o>IWDG_xPrescaler <0=>Prescaler_4 <1=>Prescaler_8 <2=>Prescaler_16
//	<3=>Prescaler_32 <4=>Prescaler_64 <5=>Prescaler_128	<6=>Prescaler_256
//<i>看门狗时钟频率 = (主频32khz/xsetr)/IWDG_xPrescaler
#define IWATCHDOG_XPRES	3
//<o>IWDG_xSetReload <0x0-0x0FFF>
//<i>看门狗定时时0x0<0x0-0x0FFF> (0-4096)
#define IWATCHDOG_XSETR 500
// </e>

/******************************************************************
			   GPIO外部中断
*******************************************************************/
//#################################################################################
// <e>IO外部中断
#define GPIO_EXTI_INTERFACE_ENABLE			  1
//</e>
/******************************************************************
				GPIO
*******************************************************************/
//#################################################################################
// <e>普通IO

// <e> GPIO1(继电1)
//==============================
// <i> Enable or disable GPIO1
#define ENABLE_GPIO1			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO1          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO1          9

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO1          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO1          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO1			1
// </e>


// <e> GPIO2(继电2)
//==============================
// <i> Enable or disable GPIO2
#define ENABLE_GPIO2			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO2          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO2          8

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO2          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO2          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO2			1
// </e>


// <e> GPIO3(继电3)
//==============================
// <i> Enable or disable GPIO3
#define ENABLE_GPIO3			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO3          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO3          7

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO3          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO3          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO3			1
// </e>


// <e> GPIO4(继电4)
//==============================
// <i> Enable or disable GPIO4
#define ENABLE_GPIO4			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO4          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO4          6

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO4          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO4          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO4			1
// </e>


// <e> GPIO5(继电5)
//==============================
// <i> Enable or disable GPIO5
#define ENABLE_GPIO5			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO5          1

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO5          15

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO5          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO5          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO5			1
// </e>


// <e> GPIO6(语音忙)
//==============================
// <i> Enable or disable GPIO6
#define ENABLE_GPIO6			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO6          1

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO6          4

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO6          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO6          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO6			1
// </e>


// <e> GPIO7(继电7)
//==============================
// <i> Enable or disable GPIO7
#define ENABLE_GPIO7			  0
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO7          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO7          6

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO7          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO7          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO7			1
// </e>


// <e> GPIO8(继电8)
//==============================
// <i> Enable or disable GPIO8
#define ENABLE_GPIO8			  0
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO8          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO8          7

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO8          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO8          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO8			1
// </e>


// <e> GPIO9(继电9)
//==============================
// <i> Enable or disable GPIO9
#define ENABLE_GPIO9			  0
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO9          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO9          8

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO9          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO9          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO9			1
// </e>


// <e> GPIO10(继电10)
//==============================
// <i> Enable or disable GPIO10
#define ENABLE_GPIO10			  0
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_GPIO10          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_GPIO10          9

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_GPIO10          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_GPIO10          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_GPIO10			1
// </e>




// </e>

/********************************************************************************
							LED
********************************************************************************/
// ####################################################################################
// <e>灯
// <i> 配置led端口
#define LED_INTERFACE_ENABLE 0

// <o> 亮灯电平<0=>低 <1=>高
// <i> 灯亮时的电平
#define LED_ON_LEVEL 1


// <e> LED1(运行灯)
//==============================
// <i> Enable or disable LED1
#define ENABLE_LED1			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_LED1          3

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_LED1          13

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_LED1          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_LED1          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_LED1			1
// </e>


// <e> LED2(异常灯)
//==============================
// <i> Enable or disable LED2
#define ENABLE_LED2			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_LED2          3

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_LED2          14

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_LED2          0x10

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_LED2          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_LED2			1
// </e>

//</e>


/********************************************************************************
							KEY
********************************************************************************/
// ####################################################################################
// <e>按键
#define KEY_INTERFACE_ENABLE 1

//<e> 按键组1
#define G1_KEY_ENABLE 1
// <o> 按下电平 <0=>低 <1=>高
// <i> 按下按键时的电平
#define G1_KEYDOWN_LEVEL 0

// <e> G1_KEY1
//==============================
// <i> Enable or disable G1_KEY1
#define ENABLE_G1_KEY1			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G1_KEY1          1

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G1_KEY1          12

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G1_KEY1          0x48

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G1_KEY1          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G1_KEY1			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G1_KEY1  1
// </e>

// <e> G1_KEY2
//==============================
// <i> Enable or disable G1_KEY2
#define ENABLE_G1_KEY2			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G1_KEY2          1

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G1_KEY2          13

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G1_KEY2          0x48

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G1_KEY2          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G1_KEY2			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G1_KEY2  2
// </e>



// <e> G1_KEY3
//==============================
// <i> Enable or disable G1_KEY3
#define ENABLE_G1_KEY3			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G1_KEY3          1

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G1_KEY3          14

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G1_KEY3          0x48

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G1_KEY3          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G1_KEY3			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G1_KEY3  3
// </e>


// <e> G1_KEY4
//==============================
// <i> Enable or disable G1_KEY4
#define ENABLE_G1_KEY4			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G1_KEY4          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G1_KEY4          0

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G1_KEY4          0x48

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G1_KEY4          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G1_KEY4			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G1_KEY4  4
// </e>


// <e> G1_KEY5
//==============================
// <i> Enable or disable G1_KEY5
#define ENABLE_G1_KEY5			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G1_KEY5          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G1_KEY5          1

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G1_KEY5          0x48

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G1_KEY5          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G1_KEY5			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G1_KEY5  5
// </e>

//</e>

//<e> 按键组2
#define G2_KEY_ENABLE 1
// <o> 按下电平 <0=>低 <1=>高
// <i> 按下按键时的电平
#define G2_KEYDOWN_LEVEL 0
// <e> K1
//==============================
// <i> Enable or disable K1
#define ENABLE_G2_K1			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G2_K1          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G2_K1          10

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G2_K1          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G2_K1          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G2_K1			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G2_K1  50
// </e>

// <e> K2
//==============================
// <i> Enable or disable K2
#define ENABLE_G2_K2			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G2_K2          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G2_K2          11

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G2_K2          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G2_K2          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G2_K2			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G2_K2  51
// </e>



// <e> K3
//==============================
// <i> Enable or disable K3
#define ENABLE_G2_K3			  1
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_G2_K3          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_G2_K3          12

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_G2_K3          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_G2_K3          2


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_G2_K3			1

//		<o> KeyValue  <0=>null <1=>key1 <2=>key2 <3=>key3 <4=>key4 <5=>key5 <6=>key6 <7=>key7 <8=>key8 <9=>key9 <10=>key10 <50=>K1 <51=>K2 <52=>K3 <53=>KGPRS
//		<i> 正键值:按下 负键值:释放
//		<i> Default: 0
#define KEYVALUE_G2_K3  52
// </e>

//</e>

//</e>


/********************************************************************************
			模拟I2C
********************************************************************************/
// ####################################################################################
// <e>模拟i2c
#define I2C_INTERFACE_ENABLE 1

//<e> I2C1
//<i>	I2C1 Enable or Disable
#define I2C1_ENABLE			1
// <h> SCL1
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_SCL_I2C1          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_SCL_I2C1          3

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_SCL_I2C1          0x14

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_SCL_I2C1          3


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_SCL_I2C1			1
// </h>

// <h> SDA1
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_SDA_I2C1          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_SDA_I2C1          2

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_SDA_I2C1          0x14

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_SDA_I2C1          3


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_SDA_I2C1			1
// </h>

// </e>
// </e>

/******************************************************************
			Uart
*******************************************************************/
// <e>串口
#define UART_INTERFACE_ENABLE 1

//<e> UART1
//<i>	UART1 Enable or Disable
#define UART1_ENABLE			0

// <h> TX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART1_TX          0

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART1_TX          9

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART1_TX          0x18

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART1_TX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART1_TX			1
// </h>

// <h> RX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART1_RX          0

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART1_RX          10

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART1_RX          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART1_RX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART1_RX			1
// </h>

//<o> 片上UART <1=>UART1 <2=>UART2 <3=>UART3 <4=>UART4 <5=>UART5 <6=>UART6
#define UART1_PORT  1

// </e>



//<e> UART2
//<i>	UART2 Enable or Disable
#define UART2_ENABLE			0

// <h> TX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART2_TX          0

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART2_TX          2

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART2_TX          0x18

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART2_TX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART2_TX			1
// </h>

// <h> RX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART2_RX          0

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART2_RX          3

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART2_RX          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART2_RX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART2_RX			1
// </h>

//<o> 片上UART <1=>UART1 <2=>UART2 <3=>UART3 <4=>UART4 <5=>UART5 <6=>UART6
#define UART2_PORT  2

// </e>



//<e> UART3
//<i>	UART3 Enable or Disable
#define UART3_ENABLE			1

// <h> TX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART3_TX          1

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART3_TX          10

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART3_TX          0x18

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART3_TX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART3_TX			1
// </h>

// <h> RX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART3_RX          1

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART3_RX          11

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART3_RX          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART3_RX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART3_RX			1
// </h>

//<o> 片上UART <1=>UART1 <2=>UART2 <3=>UART3 <4=>UART4 <5=>UART5 <6=>UART6
#define UART3_PORT  3

// </e>


//<e> UART4
//<i>	UART4 Enable or Disable
#define UART4_ENABLE			0

// <h> TX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART4_TX          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART4_TX          10

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART4_TX          0x18

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART4_TX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART4_TX			1
// </h>

// <h> RX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART4_RX          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART4_RX          11

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART4_RX          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART4_RX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART4_RX			0
// </h>

//<o> 片上UART <1=>UART1 <2=>UART2 <3=>UART3 <4=>UART4 <5=>UART5 <6=>UART6
#define UART4_PORT  4

// </e>


//<e> UART5
//<i>	UART5 Enable or Disable
#define UART5_ENABLE			1

// <h> TX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART5_TX          2

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART5_TX          12

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART5_TX          0x18

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART5_TX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART5_TX			1
// </h>

// <h> RX
//==============================
//   <i> Local GPIO define
//     <o>Port <0=>	GPIOA	<1=> GPIOB	<2=> GPIOC
//			   <3=>	GPIOD	<4=> GPIOE	<5=> GPIOF  <6=> GPIOG
//     <i> GPIO A,B,C,D...<->(0,1,2,3...)
//     <i> 0=GPIOA,1=GPIOB,...
//     <i> Default: GPIOA
#define PORT_UART5_RX          3

//     <o>Pin <0=> PIN0  <1=> PIN1  <2=> PIN2  <3=> PIN3  <4=> PIN4
//			  <5=> PIN5  <6=> PIN6  <7=> PIN7  <8=> PIN8  <9=> PIN9
//			  <10=> PIN10  <11=> PIN11  <12=> PIN12  <13=> PIN13
//			  <14=> PIN14  <15=> PIN15
//     <i> 0=PIN0,1=PIN1,...
//     <i> Default: PIN0
#define PIN_UART5_RX          2

//     <o>Mode <0x0=>  GPIO_Mode_AIN	     <0x04=> GPIO_Mode_IN_FlOATING
//			       <0x28=> GPIO_Mode_IPD   	   <0x48=> GPIO_Mode_IPU
//						 <0x14=> GPIO_Mode_Out_OD    <0x10=> GPIO_Mode_Out_PP
//						 <0x1C=> GPIO_Mode_AF_OD     <0x18=> GPIO_Mode_AF_PP
#define MODE_UART5_RX          0x04

//     <o>Speed 
//        <1=> GPIO_Speed_10MHz
//				<2=> GPIO_Speed_2MHz
//				<3=> GPIO_Speed_50MHz
//     <i> Default: GPIO_Speed_2MHz
#define SPEED_UART5_RX          1


//		<o>Init_state <0=> LOW  <1=> HIGH
//		<i> 0=LOW
//		<i> 1=HIGH
//		<i> Default: LOW
#define STATE_UART5_RX			1
// </h>

//<o> 片上UART <1=>UART1 <2=>UART2 <3=>UART3 <4=>UART4 <5=>UART5 <6=>UART6
#define UART5_PORT  5

// </e>

// </e>



// <h>BootLoader
//########################################################################
// <o> 	Bootloader_size <0x0000-0x20000>
//		<i>no bootloader should 0x0000
//	   <i>Default: 0x2000
#define _BOOTLOADER_SIZE	 0x0000//0x0000//


#endif







