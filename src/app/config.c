
#include "includes.h"

/*ͨ�����ýӿ�*/
// gpio �˿ں�      ��ŷ� �ڱۿ��� ��λ����  ���ⷢ��  �������   led��      ���ʹ��  ���������     sda       scl
#define CCM(ch)    {(ch-1)*10+3,(ch-1)*10+4,(ch-1)*10+5,(ch-1)*10+6,(ch-1)*10+7,(ch-1)*10+8,(ch-1)*10+9,(ch-1)*10+10,(ch-1)*10+11,(ch-1)*10+12},
	
ChannelConfigureMap channel_config_map[]={
		CCM(1) CCM(2) CCM(3) CCM(4) CCM(5)
};





//<e>����25/50�ڻ�
#define APP_USING_25_50_MACHINE 0
#if APP_USING_25_50_MACHINE>0
//<s>PCB����
#define PCB_VERSION    "(LD-MZ-DRIVER-5-A-V6)"

//<o>Ӳ�����<0-0xFFFF:1>
//<i>�������Ӳ���汾 0203
#define HARDWARE_VERSION          0x0203
//<o>������<0-0xFFFF:1>
//<i>�����Ŵ�0001��ʼ����
#define SOFTWARE_VERSION          0x000F

//<o>�ֵ�����
#define CHANNEL_MAX           5
#endif
//</e>