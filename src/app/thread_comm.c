#include "includes.h"
#include "packet.h"


HPacket hpacket;
/*===================================================
                本地函数
====================================================*/
/*通道处理函数*/
static void process_com(HPacket*hp)
{


}


/*发送心跳*/
void com_send_tick(HPacket*hp)
{
	packet*p = &hp->p;
  U8*data = p->data;
	data[0]=system.addr485;
	data[1]=0x05;//仓道数量
	data[2]=0x02;//硬件版本
	data[3]=0x02;//硬件版本
	
	//软件编号
  {
		U16 sv = 0;
		rw("sver",(U8*)&sv,2,'r');
		data[4] = sv>>8;
		data[5] = sv&0xFF;
	}
	
	//
}



/*===================================================
                全局函数
====================================================*/
static struct etimer et_comm;
PROCESS(thread_comm, "通讯任务");
PROCESS_THREAD(thread_comm, ev, data)  
{
	PROCESS_BEGIN();
  memset(&hpacket,0,sizeof(hpacket));
	os_delay(et_comm,1000);//延时1秒再通讯       
	ld_uart_open(COM_485,115200,8,0,1,0,1050);
	disable_485_tx();//允许接收
	while(1)
	{
		PROCESS_WAIT_EVENT();
		//接收到一个有效的数据包
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)
		{
			//处理数据包
		  
			memset(&hpacket,0,sizeof(hpacket));
			
			//打开接收
			disable_485_tx();
		}
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(thread_comm);