#include "includes.h"

//Ψһȫ�ֱ�����������
System system;
/*===================================================
                ȫ�ֺ���
====================================================*/

/*ϵͳ���ݳ�ʼ��*/
void ld_system_init(void)
{
	//��ַ�ָ� --��flash�ж�ȡ
	ld_flash_read(0, (U8*)&system.addr485, 6,RS485_ADDR);//��ȡ485��ַ
	
	//�Ƿ�֧��IR,��֧��IRʱ��ֻʹ��IIC
	#ifdef NOT_USING_IR
	system.iic_ir_mode = SIIM_ONLY_IIC; 
	#else
	system.iic_ir_mode = SIIM_IIC_IR;                    //Ĭ��iic ir��֧��
	#endif
	
	channel_addr_set((U8*)system.addr_ch);               //����ַ���õ�Channel������    

  //���ʹ��:Ĭ��:ð���Ŷӣ�ʹ�����
  system.mode = 0;
	system.enable = TRUE;
	memset(system.chs,0,CHANNEL_MAX);
}


/*ϵͳ����:mode: 100ms  500ms 2000ms*/
void ld_system_flash_led(int mode,int seconds)
{
  system.led_flash_mode = mode;
	system.led_flash_total_time = seconds*1000;
}


/*ϵͳ�ƶ�ʱ������*/
void ld_system_led_timer(int ms)
{
	system.led_flash_time+=ms;	
  system.led_flash_total_time-=ms;	
	if(system.led_flash_time>system.led_flash_mode)
	{
		system.led_flash_time=0;
		ld_gpio_set(1,!ld_gpio_get(1));
	}
	//��˸ʱ�䵽��Ϊ 2000 ����˸
	if(system.led_flash_total_time<=0)
	{
		system.led_flash_mode = 2000;
		system.led_flash_total_time=3600000;
	}
}


