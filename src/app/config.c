
#include "includes.h"

/*ͨ�����ýӿ�*/
// gpio �˿ں�      ��ŷ� �ڱۿ��� ��λ����  ���ⷢ��  �������   led��      ���ʹ��  ���������     sda       scl
#define CCM(ch)    {(ch-1)*10+3,(ch-1)*10+4,(ch-1)*10+5,(ch-1)*10+6,(ch-1)*10+7,(ch-1)*10+8,(ch-1)*10+9,(ch-1)*10+10,(ch-1)*10+11,(ch-1)*10+12},
ChannelConfigureMap channel_config_map[]={CCM(1) CCM(2) CCM(3) CCM(4) CCM(5)};

