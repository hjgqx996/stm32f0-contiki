

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

//<h>/*����оƬ*/
//</h>
typedef const  struct
{
	  int pulse;       //����ʱ��
    t_gpio_map rst;  //��λ�˿�
    t_gpio_map data; //���ݶ˿�
} t_nvxxxA_map;

//<h>/*Ƭ��spi��������  spi flash ר��*/
//</h>
typedef const  struct
{
    t_gpio_map mosi;  		//mosi �˿�
    t_gpio_map miso;  		//miso �˿�
		t_gpio_map sclk;  	//sclk �˿�
		t_gpio_map nss;   	//nss  �˿�
    char port;       			//Ƭ��spi
		U32 speed;       	//�ٶȷ�Ƶ
	  U8 sckmode;      		// 0 :low  1:high
	  U8 latchmode;    		// �������� 0 : ����  1��˫��
	  U8 master_slave;		//���� :0 �� 1����
} t_spif_map;


//<h> /*spi flash ��������*/
//</h>
typedef const struct{
	
	U8  spi;          /*ʹ�õ�spi�豸��*/
	U32 PageSize;      /*ҳ��С*/
	U32 PagesPerSector;/*����ҳ��*/
	U32 PagesPerBlock; /*����ҳ��*/
	U32 Blocks;        /*�ܿ���*/
}t_spi_flash_map;

//<h>/*pwm��������*/
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



//<h>/*uart ��������*/
//</h>
typedef const  struct
{
    t_gpio_map gpio_tx;  //tx �˿�
    t_gpio_map gpio_rx;  //rx �˿�
    char port;       		//Ƭ�� uart �˿� 0-n :uart(1)--->uart(n+1)
	
} t_uart_map;


//<h>/*adc ��������*/
//</h>
/*adc gpio ͨ��*/
typedef const struct
{
	t_gpio_map    		gpio;            					//�˿�����
    	U32  		channel;      	  				//adc channel[0-18]
} t_adc_gpio_map;
typedef struct{
  int over;   //��ѹ����
	int lower;  //��ѹ����
}t_adc_flag;
typedef const struct
{
	//adc �˿�[1-3 :ADC1=ADC3]
	U8 		adc;   

	//adc �ܷ�Ƶ[2,4,6,8]
	U32 		adc_rcc_div;
	
	//dma ��������
	U32 	dma_adc_cyc;  				/*��������,0-7*/
	U32   dma_adc_towSampleDelay;         /*���β������*/
	U16* 	dma_adc_buffer; 				/*dma����*/
	U32 	dma_adc_buffer_size; 			/*��������С,�������ݵĻ���,bytes = һ�β������� * adc_number*/
	U32   adc_number;				        /*��ǰ���õ�adc�˿ڸ���*/
	U32   adc_points;              /*��ͨ����������*/
	
	//��ʱ������
	U32   timer_pre;           /*Ԥ��Ƶ*/
	U32   timer_per;           /*����*/

	//�������ݼ��㱣��ֵ
	U64*	adc_value;
	
	t_adc_flag*flag;   //��־
	 /*����ͨ����*/
	t_adc_gpio_map*  ios;   

} t_adc_map;
#endif


//<h>/*smartcard ��������*/
//</h>
typedef const struct{
	t_gpio_map tx;
	t_gpio_map clk;
	t_gpio_map rst;
	t_gpio_map irq;
	BOOL irq_use;   //�Ƿ�ʹ��irq
	U8  port;      //Ƭ��uart[1-3,6 :USART1-USART3,USART6]
	U32 div;        //�����ʷ�Ƶ
}t_smartcard_map;


//<h> /*rc522*/
//</h>
typedef const struct
{
    //Ƭ��spi  0-2
    U8 port;
    //��λ
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
	
	//mifare���˿�
	U8 port;
	
	//��¼�������
	int record_max;	

	//�û���Ǯ����Ǯ�����ݣ���¼
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


/*��ʱ��*/
typedef const struct{
	int timer;
	int pre;
	int peroid;
}t_timer_map;


