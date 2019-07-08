

#ifndef __DRIVER_CONFIG_TYPES_H__
#define __DRIVER_CONFIG_TYPES_H__
#include "types.h"
#include "cpu.h"
//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------


//<h>/*IO配置类型*/
//</h>
typedef const struct
{
    U8 xPort;						   //端口 A~G
    U8 xPin;				   //pin脚 0~15
    U8 xMode;					   //模式
    U8 xSpeed;            //速度
    U8 xSta;					   //初始状态，高低电平
} t_gpio_map;

//<h>/*数码管显示*/
//</h>
typedef enum{
	TMT_1640 = 1,
	TMT_1688 = 2,
	TMT_1620 = 2,
}TMxxxxType;
typedef const struct{
	t_gpio_map din;
	t_gpio_map clk;
	t_gpio_map nss;
	char*      data;//行列对应的编码
	U8 counter;  //一行多数个
	U8 lines;    //行数
	TMxxxxType type;     //类型,tm1640,tm1688
	U8 mode;             //三线驱动器有显示模式
	U8 light;            //亮度
} t_tmxxxx_map;


//<h>/*eeprom*/
//</h>
typedef const struct{
	U8  port;          //i2c 端口
	int address;      //芯片地址
	int pageSize;     //页大小 bytes
	int size;         //容量 bytes
}t_eeprom_map;
//<h>/*key配置类型*/
//</h>
typedef const struct
{
		t_gpio_map    gpio;        //gpio配置
    U32  keyvalue;    // 键值
} t_key_map;
//分组按键
typedef const struct{
	t_key_map*map;     //按键组
	int number;        //组内按键个数
	int keydown;       //按下电平
} t_key_maps;


//<h>/*模拟i2c配置类型*/
//</h>
typedef const  struct
{
    t_gpio_map scl;  //scl 端口
    t_gpio_map sda;  //sda 端口
}   t_i2c_map;

typedef const struct
{
	t_gpio_map scl; //scl
	t_gpio_map sda; //sda
	U8*dir;         //方向
}t_i2cx_map;

//<h>/*语音芯片*/
//</h>
typedef const  struct
{
	  int pulse;       //脉冲时间
    t_gpio_map rst;  //复位端口
    t_gpio_map data; //数据端口
} t_nvxxxA_map;

//<h>/*片上spi配置类型  spi flash 专用*/
//</h>
typedef const  struct
{
    t_gpio_map mosi;  		//mosi 端口
    t_gpio_map miso;  		//miso 端口
		t_gpio_map sclk;  	//sclk 端口
		t_gpio_map nss;   	//nss  端口
    char port;       			//片上spi
		U32 speed;       	//速度分频
	  U8 sckmode;      		// 0 :low  1:high
	  U8 latchmode;    		// 触发边沿 0 : 单边  1：双边
	  U8 master_slave;		//主从 :0 从 1：主
} t_spif_map;


//<h> /*spi flash 配置类型*/
//</h>
typedef const struct{
	
	U8  spi;          /*使用的spi设备名*/
	U32 PageSize;      /*页大小*/
	U32 PagesPerSector;/*扇区页数*/
	U32 PagesPerBlock; /*块区页数*/
	U32 Blocks;        /*总块数*/
}t_spi_flash_map;

//<h>/*pwm配置类型*/
//</h>
typedef const struct
{
    U16 xPort:4,
             xPin :4,
             xTim :4,
             xCh  :4;
    U32 OCx;
    U32 Prescaler;
    U32 Period;
    U32 Pulse;
	  int PulseLoss;
} t_pwm_map;



//<h>/*uart 配置类型*/
//</h>
typedef const  struct
{
    t_gpio_map gpio_tx;  //tx 端口
    t_gpio_map gpio_rx;  //rx 端口
    char port;       		//片上 uart 端口 0-n :uart(1)--->uart(n+1)
	
} t_uart_map;


//<h>/*adc 配置类型*/
//</h>
/*adc gpio 通道*/
typedef const struct
{
	t_gpio_map    		gpio;            					//端口配置
    	U32  		channel;      	  				//adc channel[0-18]
} t_adc_gpio_map;
typedef struct{
  int over;   //过压计数
	int lower;  //低压计数
}t_adc_flag;
typedef const struct
{
	//adc 端口[1-3 :ADC1=ADC3]
	U8 		adc;   

	//adc 总分频[2,4,6,8]
	U32 		adc_rcc_div;
	
	//dma 常规配置
	U32 	dma_adc_cyc;  				/*采样周期,0-7*/
	U32   dma_adc_towSampleDelay;         /*两次采样间隔*/
	U16* 	dma_adc_buffer; 				/*dma缓冲*/
	U32 	dma_adc_buffer_size; 			/*缓冲区大小,所有数据的缓冲,bytes = 一次采样点数 * adc_number*/
	U32   adc_number;				        /*当前配置的adc端口个数*/
	U32   adc_points;              /*单通道采样点数*/
	
	//定时器触发
	U32   timer_pre;           /*预分频*/
	U32   timer_per;           /*周期*/

	//采样数据计算保存值
	U64*	adc_value;
	
	t_adc_flag*flag;   //标志
	 /*具体通道口*/
	t_adc_gpio_map*  ios;   

} t_adc_map;
#endif


//<h>/*smartcard 配置类型*/
//</h>
typedef const struct{
	t_gpio_map tx;
	t_gpio_map clk;
	t_gpio_map rst;
	t_gpio_map irq;
	BOOL irq_use;   //是否使用irq
	U8  port;      //片上uart[1-3,6 :USART1-USART3,USART6]
	U32 div;        //波特率分频
}t_smartcard_map;


//<h> /*rc522*/
//</h>
typedef const struct
{
    //片上spi  0-2
    U8 port;
    //复位
    t_gpio_map rst;
    
}t_rc522_map;

//<h>/*mifare*/
//</h>
typedef const struct{
	int start_sector;
	int start_offset;
	int keyA;
	int keyB;
}t_mifare_item;
typedef const struct{
	
	//mifare卡端口
	U8 port;
	
	//记录最大条数
	int record_max;	

	//用户，钱包，钱包备份，记录
	t_mifare_item user;
	t_mifare_item wallet;
	t_mifare_item walletbak;
	t_mifare_item record;
	
}t_mifare_map;


/*exti*/
typedef const struct{
	U8 port;
	U8 pin;
	U32 line;       
	U32 trigger;
}t_exti_map;


/*定时器*/
typedef const struct{
	int timer;
	int pre;
	int peroid;
}t_timer_map;


