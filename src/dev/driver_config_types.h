

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



//<h>/*uart 配置类型*/
//</h>
typedef const  struct
{
    t_gpio_map gpio_tx;  //tx 端口
    t_gpio_map gpio_rx;  //rx 端口
    char port;       		//片上 uart 端口 0-n :uart(1)--->uart(n+1)
	
} t_uart_map;



//<h>/*74h595*/
typedef const struct
{
	 t_gpio_map clk;
	 t_gpio_map cs;
	 t_gpio_map data;
}t_74hc595_map;



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

#endif


