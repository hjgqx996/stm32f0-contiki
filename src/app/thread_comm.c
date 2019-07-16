#include "includes.h"
#include "packet.h"


HPacket hpacket;

/*===================================================
                ���غ���
====================================================*/
//Ӧ��
static void send_ack(HPacket*hp, U8 cmd,U8 result)
{
	U8 send_buf[2];
	send_buf[0]=0x00;//��������
	send_buf[1]=result;//������
	packet_send(hp,cmd,2,send_buf);
}
/*===================================================
                �����������
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
			data[offset++]=ch->Voltage>>8;
			data[offset++]=ch->Voltage&0xff;//��о��ѹ
			data[offset++]=ch->bao_output; //��籦�����־ 
		}
		packet_send(hp,PC_HEART_BREAK,offset,data);
	}
}
/*�������*/
static void com_lease(HPacket*hp)
{
	//�������ݰ�����
	
	//���ȴ�ʱ������
	
	//����Ӧ��
	send_ack(hp,0x02,0x01);
	
	//��籦�Ƿ���Ч
	
	//��籦����Ƿ�һ��
	
	//6��������
	
	//������ŷ���������
	
	//���ز������

}

/*�黹����*/
static void com_return(HPacket*hp)
{
   //��������
	
	 //����
	
	//����Ӧ��
	send_ack(hp,0x03,0x01);
	
	//�����黹���첽����
}
/*�黹�첽����*/
static void com_return_async(void)
{
  //��ѯ�Ƿ�黹��ʱ
	
	//���͹黹������ݰ�

}




/*��������*/
static void com_ctrl(HPacket*hp)
{

}

/*����̼�����*/
static void com_update_entry(HPacket*hp)
{

}

/*����ģʽ��ѯ*/
static void com_update_mode(HPacket*hp)
{

}


/*ͨ��������*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	//��ַ����
	//if(p->addr!=system.addr485)
	
	switch(p->cmd)
	{
		case PC_HEART_BREAK	:		com_send_tick(hp);		break;
		case PC_LEASE				:		com_lease(hp);	    	break;
		case PC_RETURN			:		com_return(hp);				break;
		case PC_CTRL				:		com_ctrl(hp);					break;
		case PC_UPDATE_ENTRY:		com_update_entry(hp);	break;
		case PC_UPDATE_MODE :		com_update_mode(hp);	break;
		
		//���ݷ���bootloader�������ﲻ����
		case PC_UPDATE_DATA:
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
		
		//�ȴ��¼�����ʱ50ms
		wait_event(et_comm,50);
		
		//���յ�һ����Ч�����ݰ�
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)
		{
			//�������ݰ�
		  com_process(&hpacket);
			memset(&hpacket,0,sizeof(hpacket));
		}
		
		//�첽����黹����(��������ʱ,�������ݿ���������)
		com_return_async();
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(thread_comm);