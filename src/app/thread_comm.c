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
	data[0]=0x01;		//������
	data[1]=s;			//������
	data[2]=ch_addr;//��籦λ��
	memcpy(data+3,ch->id,CHANNEL_ID_MAX);
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data);
}
//���ع黹״̬
static void send_return_state(HPacket*hp,U8 s,U8 ch_addr)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	data[0]=0x01;   //������
	data[1]=ch_addr;//��籦λ��
	data[2]=s;      //������
  data[3]=ch_addr;//�ֵ����	
	data[4]=*((U8*)(&ch->state)); //�ֵ�״̬
	data[5]=*((U8*)(&ch->warn));  //�ֵ��澯
	data[6]=*((U8*)(&ch->error)); //�ֵ�����
	memcpy(data+7,ch->id,10);     //���
  data[17]=ch->Ver;             //��籦�汾
	data[18]=(U8)(ch->AverageCurrent>>8);//�洢������8λ
	data[19]=(U8)(ch->AverageCurrent&0x00ff);
	data[20]=ch->Ufsoc;//����
	data[21]=ch->Temperature;//�¶�
	data[22]=(U8)(ch->CycleCount>>8);//�洢ѭ��������8λ
	data[23]=(U8)(ch->CycleCount&0x00ff);	 
	data[24]=(U8)(ch->RemainingCapacity>>8);//�洢��о������8λ
	data[25]=(U8)(ch->RemainingCapacity&0x00ff);	 
	data[26]=(U8)(ch->Voltage>>8);//�洢��о��ѹ��8λ
	data[27]=(U8)(ch->Voltage&0x00ff);	 
	data[28]=ch->bao_output;//��籦��־��05����06��ֹ	
	packet_send(hp,0x03,29,data);
}

//�ظ�������״̬
void send_ctrl_state(HPacket*hp,U8 ctrl,U8 s)
{
	U8 len;
	U8*data=hp->p.data;
	data[0]=0x01;//������
	data[1]=ctrl;//��������
	data[2]=s;//������
  packet_send(hp,0x04,3,data);
}

void send_update_state(HPacket*hp,U8 s)
{
	U8 len;
	U8*data=hp->p.data;
	data[0]=s;//������
  packet_send(hp,0x05,1,data);
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
static volatile U8 lch=0;                      							//�ֵ�λ��
static volatile U32 ltimeout=0;                							//��ʱʱ��	
AUTOSTART_THREAD_WITH_TIMEOUT(comm_lease)
{
	static HPacket*hp;
	static packet* p ;
	static Channel*ch;
	static int lease_unlock_timeout = 0;//������ʱ
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
			
			system.state = LEASE_STATE_LEASE;//ϵͳ״̬:���
			lch = p->data[0];      //��������:���λ��
			ltimeout = p->data[11];//��������:���ʱ��
		
			if(buffer_cmp(ch->id,p->data+1,CHANNEL_ID_MAX)==FALSE){send_lease_state(hp,Lease_differ,lch);continue;}//��籦��Ų���
			
			if(ch->state.read_ok)//��籦�Ƿ���Ч
			{
//				is_ver_bt_6(ch->id)//6��������
//				{
					//����һ��
//					lease_unlock_timeout=0;
//					while(channel_read_busy(channel_data_get_index(ch),ch->iic_ir_mode))
//					{
//						os_delay(lease,10);
//						lease_unlock_timeout++;
//						if(lease_unlock_timeout>50)goto LEASE_UNLOCK_ERROR;
//					}
//					if(!channel_read_start(channel_data_get_index(ch),ch->iic_ir_mode,ch->iic_dir,RC_UNLOCK))
//						goto LEASE_UNLOCK_ERROR;
//					
//					lease_unlock_timeout=0;
//          while(channel_read_busy(channel_data_get_index(ch),ch->iic_ir_mode))
//					{
//						os_delay(lease,10);
//						lease_unlock_timeout++;
//						if(lease_unlock_timeout>50)goto LEASE_UNLOCK_ERROR;
//					} 
//				  {
//						U8 buf[2];
//						if(channel_read_end(channel_data_get_index(ch),ch->iic_ir_mode,buf)!=2)goto LEASE_UNLOCK_ERROR;
//						
//						//ir-->output==buf[0]
//						if(ch->iic_ir_mode==RTM_IR && ((buf[0]==0x05)||buf[1]==0x07))
//							goto LEASE_NEXT;
//						else
//							goto LEASE_UNLOCK_ERROR;
//						
//						//iic->outputӦ�ö�һ��
//						
//					} 
//					//�������ɹ�
//					LEASE_UNLOCK_ERROR:
//						send_lease_state(hp,Lease_decrypt_fall,lch);continue;
//				}
				LEASE_NEXT:
				channel_led_flash(channel_data_get_index(ch),ltimeout);//���ȴ�ʱ������	
				channel_discharge_all(500);     //�ر����
				dian_ci_fa(ch,HIGH);
				os_delay(comm_lease,500);
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
static volatile U8 rch=0;                      							//�ֵ�λ��
static volatile U32 rtimeout=0;                							//��ʱʱ��	
static void com_return(HPacket*hp)
{
	packet *p = &hp->p;
	Channel*ch = channel_data_get_by_addr(p->data[0]);
	
  //��������
	system.state = LEASE_STATE_RETURN;//ϵͳ״̬:�黹
	rch = p->data[0];      //��������:���λ��
	rtimeout = p->data[11];//��������:���ʱ��
	
	channel_led_flash(rch,rtimeout);//����
	send_ack(hp,0x03,0x01);         //����Ӧ��
	//�����黹���첽����
}
/*�黹�첽����*/
static void com_return_async(void)
{
  //��ѯ�Ƿ�黹��ʱ
	
	//���͹黹������ݰ�
  
}

/*��������*/
AUTOSTART_THREAD_WITH_TIMEOUT(comm_ctrl)
{
	static HPacket*hp;
	static packet* p ;
	static Channel*ch;
	static U8 cmd=0;
	static U8 bao_id[10];
	static U8 ctrl_time;
	PROCESS_BEGIN()
	while(1)
	{
		PROCESS_WAIT_EVENT();
		if(ev== PROCESS_EVENT_COMM_CTRL && data !=NULL)
		{
			hp = (HPacket*)data;
			p = &hp->p;
			ch = channel_data_get_by_addr(p->data[1]);	
			cmd=p->data[0];
			memcpy(bao_id,&p->data[2],10);  //��籦���
			ctrl_time=p->data[12];
			send_ack(hp,0x04,0x01);         //����Ӧ��
			
			switch(cmd)
			{
				case 00://�豸����
				{
					send_ctrl_state(hp,p->data[0],Cmd_success);
					delayms(1000);
					cpu_nvic_reset();
				}
				break;
				
				case 0x01://��ά�򿪲��ţ�ʹ�ܳ��1Сʱ
				{
					//6�������ϣ�����1Сʱ
					if((bao_id[6]&0x0F)>0x06)
					{
					
					}
					//����
					channel_led_flash(channel_data_get_index(ch),ctrl_time);//���ȴ�ʱ������	
					channel_discharge_all(500);     //�ر����
					dian_ci_fa(ch,HIGH);
					os_delay(comm_ctrl,500);
					dian_ci_fa(ch,LOW);//������ŷ�500ms
					if(KEY_BAI_BI(ch)==0)//�ڱۿ���
					{
						send_ctrl_state(hp,cmd,Cmd_success );channel_data_clear_by_addr(lch);//�ɹ�
					}
					else
						send_ctrl_state(hp,cmd,Cmd_fall);//��ŷ�ʧ��
				}
				
				case 0x02://ǿ�ƿ���
				{
					//����
					
					//��ŷ�
					
					//����״̬
				}
				default:
					break;
			}
		}
			continue;
	 }
		
	PROCESS_END()
}
	
/*����̼�����*/
static void com_update_entry(HPacket*hp)
{
  //�����־ 
	
	send_update_state(hp,0x01);
	cpu_ms_delay(500);
	cpu_nvic_reset();
}

/*����ģʽ��ѯ*/
static void com_update_mode(HPacket*hp)
{
  U8*data = hp->p.data;
	system.firmware_size=data[0];
	system.firmware_size<<=8;
	system.firmware_size|=data[1];
	system.firmware_size<<=8;
	system.firmware_size|=data[2];
	system.firmware_size<<=8;
	system.firmware_size|=data[3];												 
	system.firmware_version=data[4];
	system.firmware_version<<=8;
	system.firmware_version|=data[5];
	memcpy((void*)system.firmware_MD5,data+6,16);
	send_update_state(hp,normal_model);
}


/*������䴦����*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	//��ַ����
	//if(p->addr!=system.addr485)
	
	switch(p->cmd)
	{
		case PC_HEART_BREAK	:		com_send_tick(hp);		break;
		case PC_LEASE				:		process_post_synch(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);break;  //ͬ������,�¼����͸����߳�thread_lease
		case PC_RETURN			:		com_return(hp);				break;
		case PC_CTRL				:		process_post_synch(&thread_comm_ctrl,PROCESS_EVENT_COMM_CTRL,(void*)hp);break;  //ͬ������,�¼����͸����߳�thread_lease
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
                ���ݰ���������
====================================================*/
AUTOSTART_THREAD_WITHOUT_TIMEOUT(packet)
{
	PROCESS_BEGIN();
  memset(&hpacket,0,sizeof(hpacket));         //������0 	                          
	ld_uart_open(COM_485,115200,8,0,1,0,1050);  //�򿪴���
	enable_485_rx();                            //ʹ�ܽ���
	while(1)
	{	
		PROCESS_WAIT_EVENT();                           //�ȴ��жϽ��շ������¼�(�ɹ�����һ�����ݰ��¼�)
		if(data==&hpacket.p && ev==PROCESS_EVENT_PACKET)//���յ�һ����Ч�����ݰ�
		{		
		  com_process(&hpacket);												//�������ݰ�
			memset(&hpacket,0,sizeof(hpacket));           //������0
		}
	}
	PROCESS_END();
}


AUTOSTART_THREAD_WITH_TIMEOUT(comm)
{
	PROCESS_BEGIN();
	os_delay(comm,500);
	while(1)
	{	
		com_return_async();   //�黹�첽����
		os_delay(comm,80);
	}
	PROCESS_END();
}


