#include "includes.h"

//Ψһȫ�ֱ�����������
System system;
/*===================================================
                ȫ�ֺ���
====================================================*/

/*ϵͳ���ݳ�ʼ��*/
void ld_system_init(void)
{
	//��ַ�ָ� 
	system.addr485 = 0x0c;
	system.addr_ch[0] = 1;
	system.addr_ch[1] = 3;
	system.addr_ch[2] = 5;
	system.addr_ch[3] = 7;
	system.addr_ch[4] = 9;
	system.iic_ir_mode = SIIM_IIC_IR;
	channel_addr_set((U8*)system.addr_ch);
	ld_system_flash_led(500);
}


/*ϵͳ����:mode: 100ms  500ms 2000ms*/
void ld_system_flash_led(int mode)
{
  system.led_flash_mode = mode;
	system.led_flash_time = 0;
}


/*ϵͳ�ƶ�ʱ������*/
void ld_system_led_timer(int ms)
{
	system.led_flash_time+=ms;		
	if(system.led_flash_time>system.led_flash_mode)
	{
		system.led_flash_time=0;
		ld_gpio_set(1,!ld_gpio_get(1));
	}
}