/*========================================================

* file name 		:	drv_config.c
* Author 			:	kaihua.yang
* Version			：	v1.00
* Date				：	11/30/2015
* Description		：	硬件驱动配置文件

*********************************************************
*与硬件有关部分在 Configure or interface给出
*********************************************************
========================================================*/
/******************************************************************
			includes
*******************************************************************/
#include "driver_config_types.h"
#include "driver_config.h"
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
   外部时钟配置
*****************/
//<o> 时钟--外部晶振

const unsigned int Default_Hse_Enable  =    HSE_ENABLE;
const unsigned int Default_Hse_Value =      HSE_DEFAULT_VALUE;
const unsigned int Default_Hse_PLL_MUL =    HSE_PLL_MUL;
const unsigned int Default_Hse_SYSCLK_DIV = HSE_SYSCLK_DIV;
const unsigned int Default_Hse_AHB1_Div =   HSE_AHB1_DIV;//AHB1 分频
const unsigned int Default_Hse_AHB2_Div =   HSE_AHB2_DIV;//AHB2 分频

//<o> 内部时钟
const unsigned int Default_HSI_Enable  =    HSI_ENABLE;
const unsigned int Default_HSI_Value =      HSI_DEFAULT_VALUE;
const unsigned int Default_HSI_PLL_MUL =    HSI_PLL_MUL;
const unsigned int Default_HSI_SYSCLK_DIV = HSI_SYSCLK_DIV;
const unsigned int Default_HSI_AHB1_Div =   HSI_AHB1_DIV;//AHB1 分频
const unsigned int Default_HSI_AHB2_Div =   HSI_AHB2_DIV;//AHB2 分频


/*****************
   IDWG 常量配置
*****************/
//<o> iwdg
const unsigned int iwatchdog_xpres =  IWATCHDOG_XPRES;
const unsigned int iwatchdog_xsetr =  IWATCHDOG_XSETR;

/***********************
			GPIO  配置常量
***********************/
//<o> gpio
t_gpio_map gpio_map[]=
{
#if	ENABLE_GPIO1
    cpu_gpio_map(GPIO1)
#endif

#if	ENABLE_GPIO2
    cpu_gpio_map(GPIO2)
#endif

#if	ENABLE_GPIO3
    cpu_gpio_map(GPIO3)
#endif

#if	ENABLE_GPIO4
    cpu_gpio_map(GPIO4)
#endif

#if	ENABLE_GPIO5
    cpu_gpio_map(GPIO5)
#endif

#if	ENABLE_GPIO6
    cpu_gpio_map(GPIO6)
#endif

#if	ENABLE_GPIO7
    cpu_gpio_map(GPIO7)
#endif

#if	ENABLE_GPIO8
    cpu_gpio_map(GPIO8)
#endif

#if	ENABLE_GPIO9
    cpu_gpio_map(GPIO9)
#endif

#if	ENABLE_GPIO10
    cpu_gpio_map(GPIO10)
#endif

#if	ENABLE_GPIO11
    cpu_gpio_map(GPIO11)
#endif

#if	ENABLE_GPIO12
    cpu_gpio_map(GPIO12)
#endif

#if	ENABLE_GPIO13
    cpu_gpio_map(GPIO13)
#endif

#if	ENABLE_GPIO14
    cpu_gpio_map(GPIO14)
#endif

#if	ENABLE_GPIO15
    cpu_gpio_map(GPIO15)
#endif
    // 不使用
    0,
};
const unsigned char gpio_number = (sizeof(gpio_map)/sizeof(t_gpio_map)) - 1;

/***********************
			EXTI  配置常量
***********************/
//<o> exti
t_exti_map exti_map[]={
 {0,15,0x08000,0x0C},
 0

};
const unsigned char exti_number = (sizeof(exti_map) /sizeof(t_exti_map)) - 1;

/***********************
			LED  配置常量
***********************/
//<o> led
t_gpio_map led_map[]=
{
#if	ENABLE_LED0
    cpu_gpio_map(LED0)
#endif

#if	ENABLE_LED1
    cpu_gpio_map(LED1)
#endif

#if	ENABLE_LED2
    cpu_gpio_map(LED2)
#endif

#if	ENABLE_LED3
    cpu_gpio_map(LED3)
#endif

#if	ENABLE_LED4
    cpu_gpio_map(LED4)
#endif


    //不使用
    0,
};
const unsigned char led_number = sizeof(led_map) / sizeof(t_gpio_map) - 1;
const unsigned char led_on_level = LED_ON_LEVEL;


/***********************
	KEY 常量
**********************/
//<o> key
t_key_map key_map1[]=
{
#if KEY_INTERFACE_ENABLE
#if G1_KEY_ENABLE
	
#if	ENABLE_G1_KEY1
    cpu_Key_Map(G1_KEY1)
#endif

#if	ENABLE_G1_KEY2
    cpu_Key_Map(G1_KEY2)
#endif

#if	ENABLE_G1_KEY3
    cpu_Key_Map(G1_KEY3)
#endif

#if	ENABLE_G1_KEY4
    cpu_Key_Map(G1_KEY4)
#endif

#if	ENABLE_G1_KEY5
    cpu_Key_Map(G1_KEY5)
#endif
	
	#if	ENABLE_G1_KEY6
    cpu_Key_Map(G1_KEY6)
#endif

#if	ENABLE_G1_KEY7
    cpu_Key_Map(G1_KEY7)
#endif

#if	ENABLE_G1_KEY8
    cpu_Key_Map(G1_KEY8)
#endif

#if	ENABLE_G1_KEY9
    cpu_Key_Map(G1_KEY9)
#endif

#if	ENABLE_G1_KEY10
    cpu_Key_Map(G1_KEY10)
#endif


#endif
#endif

// 不使用
    0,
};

t_key_map key_map2[]=
{
#if KEY_INTERFACE_ENABLE
#if G2_KEY_ENABLE
#if	ENABLE_G2_K1
    cpu_Key_Map(G2_K1)
#endif

#if	ENABLE_G2_K2
    cpu_Key_Map(G2_K2)
#endif

#if	ENABLE_G2_K3
    cpu_Key_Map(G2_K3)
#endif
	
#endif
#endif

// 不使用
    0,
};

t_key_maps key_maps[]={
	{
		key_map1,
		sizeof(key_map1)/sizeof(t_key_map) - 1,
		G1_KEYDOWN_LEVEL,
	},
	
	{
		key_map2,
		sizeof(key_map2)/sizeof(t_key_map) - 1,
		G2_KEYDOWN_LEVEL,
	},
	
	{0}
	
};
	
const int key_maps_number = sizeof(key_maps)/sizeof(t_key_maps) - 1;

/*************************
	I2C 常量  模拟
*************************/
//<o> 模拟i2c
t_i2c_map i2c_map[] =
{
#if I2C1_ENABLE==1
    {
        cpu_gpio_map(SCL_I2C1)
        cpu_gpio_map(SDA_I2C1)
    },
#endif
    0,
};

const unsigned char i2c_number = sizeof(i2c_map) / (sizeof(t_i2c_map)) - 1;

/*************************
			Uart
*************************/
//<o> uart
t_uart_map uart_map[] = {

#if UART1_ENABLE == 1
    {
        cpu_gpio_map(UART1_TX)
        cpu_gpio_map(UART1_RX)
        UART1_PORT,
    },
#endif
		
#if UART2_ENABLE == 1
    {
        cpu_gpio_map(UART2_TX)
        cpu_gpio_map(UART2_RX)
        UART2_PORT,
    },
#endif
		
#if UART3_ENABLE == 1
    {
        cpu_gpio_map(UART3_TX)
        cpu_gpio_map(UART3_RX)
        UART3_PORT,
    },
#endif
		
#if UART4_ENABLE == 1
    {
        cpu_gpio_map(UART4_TX)
        cpu_gpio_map(UART4_RX)
        UART4_PORT,
    },
#endif
		
#if UART5_ENABLE == 1
    {
        cpu_gpio_map(UART5_TX)
        cpu_gpio_map(UART5_RX)
        UART5_PORT,
    },
#endif
		
#if UART6_ENABLE == 1
    {
        cpu_gpio_map(UART6_TX)
        cpu_gpio_map(UART6_RX)
        UART6_PORT,
    },
#endif
    0,

};

const unsigned char uart_number = sizeof(uart_map) /(sizeof(t_uart_map)) - 1;

/*************************
			74hc595
*************************/
//<o> 74hc595
//t_74hc595_map	hc595_map[]={
//	cpu_gpio_map(HC595_CLK)
//	cpu_gpio_map(HC595_CS)
//	cpu_gpio_map(HC595_DATA)
//};

/************************
		bootloader
************************/
//<o> bootloader
const unsigned int  BOOTLOADER_SIZE = _BOOTLOADER_SIZE;

/*************************************************************
	device init
*************************************************************/
#if WIN32==0
extern void GPIO_ReleaseSWDJ(void);
extern void cpu_rcc_config(void);
//<o>dev init
void ld_dev_init(void)
{
    /*系统运行时钟*/
    //cpu_rcc_config();
	
    /*中断向量表定位，bootsize*/
    //cpu_nvic_app_Config();	
	
	  /*打印版本号*/
    gt_uart_init();

    /* 通用GPIO */
#if GPIO_INTERFACE_ENABLE
	  GPIO_ReleaseSWDJ();
    gt_io_init();
#endif
	  /* LED */
#if LED_INTERFACE_ENABLE
		gt_led_init();
#endif
	
	  /* KEY */
#if KEY_INTERFACE_ENABLE
		gt_key_init();
#endif


    /*外部中断初始化*/
    //gt_ex_irq_config(0, 12, 0, 0);//PA12 as dm9000

    /*看门狗初始化*/
#if IWDG_INTERFACE_ENABLE
//     gt_iwdg_init();
//     gt_iwdg_open();
#endif


#if GPRS_EXTI_INTERFACE_ENABLE
  gt_exti_init();
#endif

}

#endif



