

#ifndef __CONFIG_H__
#define __CONFIG_H__

//���ݰ������¼�
#define PROCESS_EVENT_PACKET     0x40

//485���ں�
#define COM_485                   2

//485����ʹ��
#define enable_485_tx()  ld_gpio_set(2,1)
#define disable_485_tx() ld_gpio_set(2,0)

#endif