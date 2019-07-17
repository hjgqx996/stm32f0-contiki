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

//�������״̬
static void send_lease_state(HPacket*hp, U8 s,U8 ch_addr)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	data[0]=0x01;//������
	data[1]=s;//������
	data[2]=ch_addr;
	memcpy(data+3,ch->id,CHANNEL_ID_MAX);
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data);
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
static volatile LeaseState ls=LEASE_STATE_INIT;              //ϵͳ���״̬
static volatile U8 lch=0;                      							//�ֵ�λ��
static volatile U32 ltimeout=0;                							//��ʱʱ��	

static struct etimer et_comm_lease;
PROCESS(thread_comm_lease, "��账�� ����");
AUTOSTART_PROCESSES(thread_comm_lease);
PROCESS_THREAD(thread_comm_lease, ev, data)  
{
	static HPacket*hp;
	static packet* p ;
	static Channel*ch;
	PROCESS_BEGIN();
	while(1){
	  PROCESS_WAIT_EVENT();//�ȴ��¼�����
		if(ev== PROCESS_EVENT_COMM_LEASE && data !=NULL)
		{
			hp = (HPacket*)data;
			p = &hp->p;
			ch = channel_data_get_by_addr(p->data[0]);
			
			send_ack(hp,0x02,0x01);//����Ӧ��
			if(ch==NULL)break;
			
			ls = LEASE_STATE_LEASE;
			lch = p->data[0];
			ltimeout = p->data[11];//�������ݰ�����
		
			if(buffer_cmp(ch->id,p->data+1,CHANNEL_ID_MAX)==FALSE){send_lease_state(hp,Lease_differ,lch);continue;}//��籦��Ų���
			
			if(ch->state.read_ok)//��籦�Ƿ���Ч
			{
				is_6_battery(ch->id)//6��������
				{
					//����һ��
					
					//��ѯ�����־ -->
					
					send_lease_state(hp,Lease_decrypt_fall,lch);continue;//�������ɹ�
				}
				channel_led_flash(lch,ltimeout);//���ȴ�ʱ������	
				//�ر����
				dian_ci_fa(ch,HIGH);
				os_delay(et_comm_lease,500);
				dian_ci_fa(ch,LOW);//������ŷ�500ms
				if(KEY_BAI_BI(ch)==0)//�ڱۿ���
				{
				  send_lease_state(hp,Lease_success,lch);channel_data_clear_by_addr(lch);//�ɹ�
				}
				else
					send_lease_state(hp,Lease_dianchifa_fall,lch);//��ŷ�ʧ��
			}
			continue;
		}
	}
	PROCESS_END();
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
		case PC_LEASE				:		process_post_synch(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);break;  //ͬ�������¼�
		case PC_RETURN			:		com_return(hp);				break;
		case PC_CTRL				:		com_ctrl(hp);					break;
		case PC_UPDATE_ENTRY:		com_update_entry(hp);	break;
		case PC_UPDATE_MODE :		com_update_mode(hp);	break;
	
		case PC_UPDATE_DATA://���ݷ���bootloader�������ﲻ����
	  default:	
			memset(hp,0,sizeof(HPacket));
      enable_485_rx();
		break;
	}			
}

/*===================================================
                ȫ�ֺ���
====================================================*/
PROCESS(thread_packet, "���ݰ���������");
AUTOSTART_PROCESSES(thread_packet);
PROCESS_THREAD(thread_packet, ev, data)  
{
	PROCESS_BEGIN();
  memset(&hpacket,0,sizeof(hpacket));
	ld_uart_init(); 	
	ld_uart_open(COM_485,115200,8,0,1,0,1050);
	enable_485_rx();
	while(1)
	{	
		PROCESS_WAIT_EVENT();
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)//���յ�һ����Ч�����ݰ�
		{		
		  com_process(&hpacket);//�������ݰ�
			memset(&hpacket,0,sizeof(hpacket));
		}
	}
	PROCESS_END();
}


static struct etimer et_comm;
PROCESS(thread_comm, "ͨѶ����");
AUTOSTART_PROCESSES(thread_comm);
PROCESS_THREAD(thread_comm, ev, data)  
{
	PROCESS_BEGIN();
	os_delay(et_comm,500);
	while(1)
	{	
		com_return_async();   //�黹�첽����
		os_delay(et_comm,80);
	}
	PROCESS_END();
}


