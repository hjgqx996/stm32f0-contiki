
#ifndef __25_50_NAS_H__
#define __25_50_NAS_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//<e>����25/50�ڻ�
#define APP_USING_25_50_MACHINE 1
//<s>PCB����
#define PCB_VERSION    "(LD-MZ-DRIVER-5-A-V6)"

//<o>Ӳ�����<0-0xFFFF:1>
//<i>�������Ӳ���汾 0203
#define HARDWARE_VERSION          0x0203
//<o>������<0-0xFFFF:1>
//<i>�����Ŵ�0001��ʼ����
#define SOFTWARE_VERSION          0x0011

//<o>�ֵ�����
#define CHANNEL_MAX           5

//</e>

//��ŷ���Դʹ��
#define dian_ci_fa_power(enable)
//5V������
#define set_out5v()     ld_gpio_set(pch->map->io_mp,1) //���5V

#endif
