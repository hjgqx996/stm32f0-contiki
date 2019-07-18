
#include "includes.h"

/*通道配置接口*/
// gpio 端口号      电磁阀 摆臂开关 到位开关  红外发送  红外接收   led灯      充电使能  充电输出检测     sda       scl
#define CCM(ch)    {(ch-1)*10+3,(ch-1)*10+4,(ch-1)*10+5,(ch-1)*10+6,(ch-1)*10+7,(ch-1)*10+8,(ch-1)*10+9,(ch-1)*10+10,(ch-1)*10+11,(ch-1)*10+12},
ChannelConfigureMap channel_config_map[]={CCM(1) CCM(2) CCM(3) CCM(4) CCM(5)};

