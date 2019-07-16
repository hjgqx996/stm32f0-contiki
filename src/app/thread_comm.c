#include "includes.h"
#include "packet.h"


HPacket hpacket;
/*===================================================
                ���غ���
====================================================*/

/*��������*/
static void com_send_tick(HPacket*hp)
{
	packet*p = &hp->p;
  U8*data = p->data;
	data[0]=system.addr485;
	data[1]=CHANNEL_MAX;//�ֵ�����
	data[2]=HARDWARE_VERSION>>8;    //Ӳ���汾
	data[3]=HARDWARE_VERSION&0x00FF;//Ӳ���汾
	
	//������
	data[4] = SOFTWARE_VERSION>>8;
	data[5] = SOFTWARE_VERSION&0x00FF;
	{
		U8 i=0;
		U8 offset=6;
		for(;i<CHANNEL_MAX;i++)
		{
			Channel*ch = channel_data_get(i+1);
			if(ch==NULL)return;
			data[offset++]=ch->addr;//�ֵ���ַ
			data[offset++]=*((U8*)(&ch->state)); //�ֵ�״̬
			data[offset++]=*((U8*)(&ch->warn));  //�ֵ��澯
			data[offset++]=*((U8*)(&ch->error));  //�ֵ�����
			memcpy(data+offset,ch->id,CHANNEL_ID_MAX);offset+=CHANNEL_ID_MAX;//�ֵ����
			data[offset++]=ch->Ver;//��籦�汾
			data[offset++]=ch->AverageCurrent>>8;
			data[offset++]=ch->AverageCurrent&0xFF;//ƽ������
			data[offset++]=ch->Ufsoc;//ʣ�����
			data[offset++]=ch->Temperature;//�¶�
			data[offset++]=ch->CycleCount>>8;
			data[offset++]=ch->CycleCount&0xff;//ѭ������
			data[offset++]=ch->RemainingCapacity>>8;
			data[offset++]=ch->RemainingCapacity&0xFF;//ʣ������
			data[offset++]=ch->bao_output; //��籦�����־ 
		}
		packet_send(hp,PC_HEART_BREAK,offset,data);
	}
}


/*ͨ��������*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	switch(p->cmd)
	{
		case PC_HEART_BREAK:
			com_send_tick(hp);
		break;
	
	  default:
			memset(hp,0,sizeof(HPacket));
		  enable_485_rx();
		break;
	}
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
		  com_process(&hpacket);
			memset(&hpacket,0,sizeof(hpacket));
		}
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(thread_comm);