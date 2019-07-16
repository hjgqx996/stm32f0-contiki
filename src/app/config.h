

#ifndef __CONFIG_H__
#define __CONFIG_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

//���ݰ������¼�
#define PROCESS_EVENT_PACKET     0x40

//<o>485���ں�  <1=>uart1 <2=>uart2
#define COM_485                   2

//485����ʹ��
#define enable_485_tx()   ld_gpio_set(2,1)
#define disable_485_tx()  ld_gpio_set(2,0)
#define enable_485_rx()   disable_485_tx()
#define disable_485_rx()  enable_485_tx()

//<o>�ֵ�����
#define CHANNEL_MAX                5

//<o>�ֵ���ų���
#define CHANNEL_ID_MAX             10

//<o>Ӳ�����<0x0-0xFFFF:4>
#define HARDWARE_VERSION          0x0202
//<o>������<0x0-0xFFFF:4>
#define SOFTWARE_VERSION          0x0001



#endif