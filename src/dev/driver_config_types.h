

#ifndef __DRIVER_CONFIG_TYPES_H__
#define __DRIVER_CONFIG_TYPES_H__
#include "types.h"
#include "cpu.h"
//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------


//<h>/*IO��������*/
//</h>
typedef const struct
{
    U8 xPort;						   //�˿� A~G
    U8 xPin;				   //pin�� 0~15
    U8 xMode;					   //ģʽ
    U8 xSpeed;            //�ٶ�
    U8 xSta;					   //��ʼ״̬���ߵ͵�ƽ
} t_gpio_map;

//<h>/*�������ʾ*/
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
	char*      data;//���ж�Ӧ�ı���
	U8 counter;  //һ�ж�����
	U8 lines;    //����
	TMxxxxType type;     //����,tm1640,tm1688
	U8 mode;             //��������������ʾģʽ
	U8 light;            //����
} t_tmxxxx_map;


//<h>/*eeprom*/
//</h>
typedef const struct{
	U8  port;          //i2c �˿�
	int address;      //оƬ��ַ
	int pageSize;     //ҳ��С bytes
	int size;         //���� bytes
}t_eeprom_map;
//<h>/*key��������*/
//</h>
typedef const struct
{
		t_gpio_map    gpio;        //gpio����
    U32  keyvalue;    // ��ֵ
} t_key_map;
//���鰴��
typedef const struct{
	t_key_map*map;     //������
	int number;        //���ڰ�������
	int keydown;       //���µ�ƽ
} t_key_maps;


//<h>/*ģ��i2c��������*/
//</h>
typedef const  struct
{
    t_gpio_map scl;  //scl �˿�
    t_gpio_map sda;  //sda �˿�
}   t_i2c_map;

typedef const struct
{
	t_gpio_map scl; //scl
	t_gpio_map sda; //sda
	U8*dir;         //����
}t_i2cx_map;



//<h>/*uart ��������*/
//</h>
typedef const  struct
{
    t_gpio_map gpio_tx;  //tx �˿�
    t_gpio_map gpio_rx;  //rx �˿�
    char port;       		//Ƭ�� uart �˿� 0-n :uart(1)--->uart(n+1)
	
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


/*��ʱ��*/
typedef const struct{
	int timer;
	int pre;
	int peroid;
}t_timer_map;

#endif


