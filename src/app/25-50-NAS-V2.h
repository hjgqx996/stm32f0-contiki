
#ifndef __25_50_NAS_H__
#define __25_50_NAS_H__

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

//<s>PCB����
#define PCB_VERSION    "(LD-MZ-DRIVER-5-A-V2)"

//<o>Ӳ�����<0-0xFFFF:1>
//<i>�������Ӳ���汾 0203
#define HARDWARE_VERSION          0x0201
//<o>������<0-0xFFFF:1>
//<i>�����Ŵ�0001��ʼ����
#define SOFTWARE_VERSION          0x0018

//<o>�ֵ�����
#define CHANNEL_MAX               5

//<e>ʹ�� 3��HC595
#define USING_3_HC595             1
//</e>

//<e>��ʹ�� IR
#define NOT_USING_IR               1
//</e>




//��ŷ���Դʹ��
#define dian_ci_fa_power(enable)
//5V������
#define set_out5v()     ld_gpio_set(pch->map->io_mp,1) //���5V

//Ӳ��io����
#define driver_config_io_map() \
	/*��ŷ� �ڱۿ��� ��λ����  ���ⷢ��  �������   led��      ���ʹ��  ���������     sda       scl*/    \
	OUT(HC,2) IN(B,10) IN(B,15)  NoIO()    NoIO()    OUT(HC,0)  OUT(HC,1)  IN(C,0)         ODH(B,8)  ODH(B,9) \
	OUT(HC,6) IN(B,1)  IN(A,8)   NoIO()    NoIO()    OUT(HC,4)  OUT(HC,5)  IN(C,1)         ODH(B,6)  ODH(B,7) \
	OUT(HC,10)IN(A,0)  IN(F,6)   NoIO()    NoIO()    OUT(HC,8)  OUT(HC,9)  IN(C,5)         ODH(B,4)  ODH(B,5) \
	OUT(HC,14)IN(C,13) IN(F,7)   NoIO()    NoIO()    OUT(HC,12) OUT(HC,13) IN(C,4)         ODH(D,2)  ODH(B,3) \
	OUT(HC,18)IN(C,9)  IN(A,12)  NoIO()    NoIO()    OUT(HC,16) OUT(HC,17) IN(B,13)        ODH(C,11) ODH(C,12)

//Ӳ��exti����
#define driver_config_exti_map()\
	/*�ֵ�1*/exti(B,15)  \
	/*�ֵ�2*/exti(A, 8)  \
	/*�ֵ�3*/exti(F, 6)  \
	/*�ֵ�4*/exti(F, 7)  \
	/*�ֵ�5*/exti(A,12)
	
	
	
#endif
