#include "includes.h"
#include "packet.h"

HPacket hpacket;


/*����һ�γ�籦*/
#define UnlockBao ReadBao

/*cmd:����  timeout:��ʱms, thread:�߳���,fail:ʧ��ʱ���еĴ���  sucess:�ɹ�ʱ���еĴ���, size:�������ݻ����С*/
#define ReadBao(cmd,timeout,thread,fail,sucess,size)  {\
	/* �������:��ʱ */  static S16 to = timeout/10;static S8 err=0; static U8 dataout[size]; \
  /* ��������      */	 do{err = channel_read(pch,cmd,dataout);to--;os_delay(thread,10); \
	/* ��ʱ�˳�*/        if(to<0){err=4;break;}}while(err<2); \
	/* �ɹ�*/            if(err==2)sucess \
	/* ʧ��*/            else fail}



/*===================================================
                ���غ���
====================================================*/
//Ӧ��
static void send_ack(HPacket*hp, U8 cmd,U8 result)
{
	U8 send_buf[2];
	send_buf[0]=0x00;//��������
	send_buf[1]=result;//������
	packet_send(hp,cmd,2,send_buf,system.addr485);
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
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data,system.addr485);
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
	packet_send(hp,0x03,29,data,system.addr485);
}

//�ظ�������״̬
static void send_ctrl_state(HPacket*hp,U8 ctrl,U8 s)
{
	U8*data=hp->p.data;
	data[0]=0x01;//������
	data[1]=ctrl;//��������
	data[2]=s;//������
  packet_send(hp,0x04,3,data,system.addr485);
}
//�ظ�����״̬
static void send_update_state(HPacket*hp,U8 s)
{
	U8*data=hp->p.data;
	data[0]=s;//������
  packet_send(hp,0x05,1,data,system.addr485);
}

//��¼���ظ�0x16
void send_handshake_state(HPacket*hp,U8 s)
{
	packet_send(hp,0x16,1,&s,0xFF);//������	 
}

//��¼���ظ�0x06
void send_add_state(HPacket*hp,U8 add,U8 s)
{
	uint8_t send_buf[2];
	send_buf[0]=add;//��������
	send_buf[1]=s;//������		
	packet_send(hp,0x06,2,&s,0xFF);//������	
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
	data[4] = SOFTWARE_VERSION>>8;    //������
	data[5] = SOFTWARE_VERSION&0x00FF;//������
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
		packet_send(hp,PC_HEART_BREAK,offset,data,system.addr485);
		memset(hp,0,sizeof(HPacket));
	}
}

/*�������*/
static volatile U8 lch=0;                      							//�ֵ�λ��
static volatile U32 ltimeout=0;                							//��ʱʱ��	
AUTOSTART_THREAD_WITH_TIMEOUT(comm_lease)
{
	static HPacket*hp;
	static Channel*pch;
	PROCESS_BEGIN();
	while(1){
	  PROCESS_WAIT_EVENT();//�ȴ��¼�����
		if(ev== PROCESS_EVENT_COMM_LEASE && data !=NULL)
		{
			hp = (HPacket*)data;                  
			pch = channel_data_get_by_addr(hp->p.data[0]);
			
			send_ack(hp,0x02,0x01);//����Ӧ��
			if(pch==NULL)break;
			
			system.state = SYSTEM_STATE_LEASE;//ϵͳ״̬:���
			lch = hp->p.data[0];      //��������:���λ��
			ltimeout = 1000*hp->p.data[11];//��������:���ʱ��(ms)
		
			if(buffer_cmp(pch->id,hp->p.data+1,CHANNEL_ID_MAX)==FALSE){send_lease_state(hp,Lease_differ,lch);continue;}//��籦��Ų���
			
			if(pch->state.read_ok)//��籦�Ƿ���Ч
			{
				if(is_ver_6() || is_ver_7())//6��������,600ms,ʧ�ܷ���Ӧ���
					UnlockBao(RC_UNLOCK,600,comm_lease,{},{send_lease_state(hp,Lease_decrypt_fall,lch);continue;},2);
					
				channel_led_flash(channel_data_get_index(pch),ltimeout);//����	
				request_charge_hangup_all(1);//�ر����,1s
				dian_ci_fa(pch,HIGH);      //��ŷ�����
				os_delay(comm_lease,500);  //��ʱ500ms
				dian_ci_fa(pch,LOW);       //�رյ�ŷ�
				if(!isvalid_baibi())       //�ڱۿ���
				{
					send_lease_state(hp,Lease_success,lch); //�ɹ�===>Ӧ���
				  channel_data_clear_by_addr(lch);        //�ɹ�===>������
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
	system.state = SYSTEM_STATE_RETURN;//ϵͳ״̬:�黹//�����黹���첽����
	rch = p->data[0];      //��������:�黹λ��
	rtimeout = p->data[11];//��������:�黹��ʱ
	
	channel_led_flash(rch,rtimeout);//����
	send_ack(hp,0x03,0x01);         //����Ӧ��
}

/*��������*/
AUTOSTART_THREAD_WITH_TIMEOUT(comm_ctrl)
{
	static HPacket*hp;
	static Channel*pch;
	static U8 cmd=0;
	static U8 bao_id[10];
	static U8 ctrl_time;
	static U8 ch_addr = 0;
	PROCESS_BEGIN()
	while(1)
	{
		PROCESS_WAIT_EVENT();
		if(ev== PROCESS_EVENT_COMM_CTRL && data !=NULL)
		{
			hp = (HPacket*)data;
			ch_addr = hp->p.data[1];
			pch = channel_data_get_by_addr(ch_addr);if(pch==NULL)continue;		
			cmd=hp->p.data[0];
			memcpy(bao_id,&hp->p.data[2],10);//��籦���
			ctrl_time=hp->p.data[12];        //����ʱ��
			send_ack(hp,0x04,0x01);          //����Ӧ��
			
			if(cmd==0x01)//�豸����
			{
				send_ctrl_state(hp,hp->p.data[0],Cmd_success);
				delayms(1000);
				cpu_nvic_reset();
				continue;
			}
				
			if(cmd==0x02)//��ά�򿪲��ţ�ʹ�ܳ��1Сʱ
			{
				if((bao_id[6]&0x0F)>0x06)//6�������ϣ�����1Сʱ
				{
					 UnlockBao(RC_UNLOCK_1HOUR,1000,comm_ctrl,{},{},2);
				}
			}
			
			if(cmd==0x02||cmd==0x03)//ǿ�ƿ���	
			{					
				channel_led_flash(channel_data_get_index(pch),ctrl_time);//����	
				request_charge_hangup_all(1);//�ر����,1s
				dian_ci_fa(pch,HIGH);      //��ŷ�����
				os_delay(comm_ctrl,500);   //��ʱ500ms
				dian_ci_fa(pch,LOW);       //�رյ�ŷ�
				if(!isvalid_baibi())       //�ڱۿ���
				{
					send_ctrl_state(hp,cmd,Cmd_success );        //�ɹ�===>����Ӧ��
					 channel_data_clear_by_addr(ch_addr);        //�ɹ�===>������
				}
				else
					send_ctrl_state(hp,cmd,Cmd_fall);//��ŷ�ʧ��
			}
		}
    memset(hp,0,sizeof(HPacket));		
		continue;
	 }
		
	PROCESS_END()
}
	
/*����̼�����*/
static void com_update_entry(HPacket*hp)
{
  //�����־ 
	system.firmware_updata_flag=0x88;
	ld_flash_write(0,&system.firmware_updata_flag,1,FIRMWARE_updata);
	send_update_state(hp,0x01);
	cpu_ms_delay(500);
	cpu_nvic_reset();//��λ
}

/*����ģʽ��ѯ*/
static void com_update_mode_query(HPacket*hp)
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
	if(p->addr==system.addr485)
		switch(p->cmd)
		{
			case PC_HEART_BREAK	:   ld_system_flash_led(2000);	com_send_tick(hp);		return;                         //ϵͳ��2000ms������������
			case PC_LEASE				:		process_post(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);return; //ͬ������,�¼����͸����߳�thread_lease
			case PC_RETURN			:		com_return(hp);				return;
			case PC_CTRL				:		process_post(&thread_comm_ctrl,PROCESS_EVENT_COMM_CTRL,(void*)hp);return;   //ͬ������,�¼����͸����߳�thread_lease
			case PC_UPDATE_ENTRY:		com_update_entry(hp);	break;
			case PC_UPDATE_MODE :		com_update_mode_query(hp);break;
			
			case PC_UPDATE_DATA://���ݷ���bootloader�������ﲻ����
			default:break;
			break;
		}	
	//��¼����ͨѶ
	else if(p->addr==0xFE)
	{
		switch(p->cmd)
		{
			case 0x06://��д��ַ
				memcpy((void*)&system.addr485,p->data,1+CHANNEL_MAX);             //���浽ϵͳ����������
			  channel_addr_set((U8*)system.addr_ch);                            //���浽�ֵ�����������
				ld_flash_write(0,(U8*)&system.addr485, 1+CHANNEL_MAX, RS485_ADDR);//д��flash����
				send_add_state(hp,system.addr485,0x01);
			break;
			case 0x016: send_handshake_state(hp,0x01); break;//����
			default:break;
		}
	}
		memset(hp,0,sizeof(HPacket));
		enable_485_rx();	
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
		  com_process(&hpacket);												//�������ݰ�
		ld_iwdg_reload();
	}
	PROCESS_END();
}

/*===================================================
                �黹�첽����
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(return)
{
	static Channel*pch;
	PROCESS_BEGIN();
	os_delay(return,500);
	
	while(1)
	{	
		if(system.state == SYSTEM_STATE_RETURN)
		{
			if(istimeout(time(0),rtimeout))//��ʱ
			{
				send_return_state(&hpacket,Return_timeout,rch);//���ͳ�ʱ�黹��
				system.state=SYSTEM_STATE_INIT;//��λ״̬
				rch=0;
				rtimeout=0;
			}else{
				pch=channel_data_get_by_addr(rch);
				if(isvalid_baibi())//�й黹��ʶ��һ�³�籦
				{
				  os_delay(return,20);//20msȥ��
					if(isvalid_baibi())
					{
						static int i=0;
						for(;i<3;i++)
						{
							//����3�ζ�ȡ��籦��Ϣ,ÿ�γ�ʱ3000ms
							if(!channel_id_is_not_null(pch->id))
								ReadBao(RC_READ_ID,3000,return,{},{memcpy(pch->id,dataout,10);break;},10);					
						}	
						if(!channel_id_is_not_null(pch->id))//�޷�ʶ��
							send_return_state(&hpacket,Return_unrecognized,rch);
						else
							send_return_state(&hpacket,Return_success,rch);
					
						pch->flash=FALSE;//����˸
						system.state=SYSTEM_STATE_INIT;//��λ
					}
				}
			}
		}
		os_delay(return,80);
		ld_iwdg_reload();
	}
	PROCESS_END();
}


