
#include "includes.h"

/*通道配置接口*/
// gpio 端口号      电磁阀 摆臂开关 到位开关  红外发送  红外接收   led灯      充电使能  充电输出检测     sda       scl
#define CCM(ch)    {(ch-1)*10+3,(ch-1)*10+4,(ch-1)*10+5,(ch-1)*10+6,(ch-1)*10+7,(ch-1)*10+8,(ch-1)*10+9,(ch-1)*10+10,(ch-1)*10+11,(ch-1)*10+12},
	
ChannelConfigureMap channel_config_map[]={
		CCM(1) CCM(2) CCM(3) CCM(4) CCM(5)
};





//<e>用于25/50口机
#define APP_USING_25_50_MACHINE 0
#if APP_USING_25_50_MACHINE>0
//<s>PCB板编号
#define PCB_VERSION    "(LD-MZ-DRIVER-5-A-V6)"

//<o>硬件编号<0-0xFFFF:1>
//<i>带红外的硬件版本 0203
#define HARDWARE_VERSION          0x0203
//<o>软件编号<0-0xFFFF:1>
//<i>软件编号从0001开始叠加
#define SOFTWARE_VERSION          0x000F

//<o>仓道个数
#define CHANNEL_MAX           5
#endif
//</e>