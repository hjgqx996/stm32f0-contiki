#include "includes.h"
#include "packet.h"


HPacket hpacket;
/*===================================================
                ���غ���
====================================================*/
/*ͨ��������*/
static void process_com(HPacket*hp)
{


}


/*��������*/
void com_send_tick(HPacket*hp)
{
	packet*p = &hp->p;
  U8*data = p->data;
	data[0]=system.addr485;
	data[1]=0x05;//�ֵ�����
	data[2]=0x02;//Ӳ���汾
	data[3]=0x02;//Ӳ���汾
	
	//������
  {
		U16 sv = 0;
		rw("sver",(U8*)&sv,2,'r');
		data[4] = sv>>8;
		data[5] = sv&0xFF;
	}
	
	//
}



/*===================================================
                ȫ�ֺ���
====================================================*/
static struct etimer et_comm;
PROCESS(thread_comm, "ͨѶ����");
PROCESS_THREAD(thread_comm, ev, data)  
{
	PROCESS_BEGIN();
  memset(&hpacket,0,sizeof(hpacket));
	os_delay(et_comm,1000);//��ʱ1����ͨѶ       
	ld_uart_open(COM_485,115200,8,0,1,0,1050);
	disable_485_tx();//�������
	while(1)
	{
		PROCESS_WAIT_EVENT();
		//���յ�һ����Ч�����ݰ�
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)
		{
			//�������ݰ�
		  
			memset(&hpacket,0,sizeof(hpacket));
			
			//�򿪽���
			disable_485_tx();
		}
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(thread_comm);