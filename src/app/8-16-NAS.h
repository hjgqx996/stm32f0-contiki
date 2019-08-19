
#ifndef __8_16_NAS_H__
#define __8_16_NAS_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//<e>����8/16�ڻ�
#define APP_USING_8_16_MACHINE 1
//<s>PCB����
#define PCB_VERSION    "(LD-MZ-DRIVER-8-A-V21.0)"

//<o>Ӳ�����<0-0xFFFF:1>
//<i>�������Ӳ���汾 0203
#define HARDWARE_VERSION          0x0203
//<o>������<0-0xFFFF:1>
//<i>�����Ŵ�0001��ʼ����
#define SOFTWARE_VERSION          0x0006

//<o>�ֵ�����
#define CHANNEL_MAX           4

//</e>

//��ŷ���Դʹ��
#define dian_ci_fa_power(enable)   ld_gpio_set(43,enable)

//5V������
#define set_out5v()      do{ld_gpio_set_io(pch->map->io_mp_detect,TRUE,1);ld_gpio_set(pch->map->io_mp,1);ld_gpio_set_io(pch->map->io_mp_detect,FALSE,0);}while(0)

#endif
