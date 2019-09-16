#include "includes.h"
#include "packet.h"

HPacket hpacket;//�������ݰ�����
/*===================================================
                ���غ���
====================================================*/
/*��ŷ�����ʱ�����ڱۿ�����Чʱ��*/
static int bai_bi_counter(Channel*pch,int timeout)
{
	int bc = 0;
	if(pch==NULL)return 0;
	while(timeout>0)
	{
		delayms(10);
		if(isvalid_baibi())
			bc++;
		timeout-=10;
	}
	return bc*10;
}
/*һ�ε�ŷ����� + ���� + ���ɹ� 
* pch           :�ֵ�����
* flash_time    :�ֵ�����˸ʱ��
* timeout       :��ŷ�����ʱ��
* hightime_min  :��ŷ�����ʱ��
* check_time    :���ʱ��
* hightime_max  :�ڱۿ��������Чʱ�� ms
* return        :TRUE or FALSE
*/
BOOL diancifa(Channel*pch,int flashtime,int timeout,int check_time,int hightime_max)
{
	int bcounter=0;
	request_charge_hangup_all(0);//�ر����,2s
	dian_ci_fa_power(1);         //ʹ�ܵ�ŷ���Դ
	delayms(10);                 //����595�ڶ�ʱ���б�ˢ�£���һ����ʱ������ȴ�����������
	dian_ci_fa(pch,HIGH);        //��ŷ�����
	delayms(500);
	dian_ci_fa(pch,LOW);         //�رյ�ŷ�
	bcounter = bai_bi_counter(pch,50);//��ŷ��򿪵�ʱ�򣬶�ȡ�ڱۿ��ص�ƽ
	dian_ci_fa_power(0);         //�رյ�ŷ���Դ
	if(bcounter < hightime_max){
		channel_led_flash(channel_data_get_index(pch),flashtime);//����	
		return TRUE;
	}
	else return FALSE;
}

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
static void send_lease_state(HPacket*hp, U8 s,U8 ch_addr,U8*ids)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	if(ch==NULL){enable_485_rx();return;}
	data[0]=0x01;		//������
	data[1]=s;			//������
	data[2]=ch_addr;//��籦λ��
	memcpy(data+3,ids,CHANNEL_ID_MAX);
	
	//�ж����ɹ���ʧ��==>��־λ
	if(s != Lease_success)
	{
		Channel*pch = channel_data_get_by_addr(ch_addr);
		if(pch!=NULL)
		{
			pch->error.lease=1;//���ʧ��λ����1
		}
	}
	else
  {
		Channel*pch = channel_data_get_by_addr(ch_addr);
		if(pch!=NULL)
		{
			pch->error.lease=0;//���ʧ��λ����0
		}	
	}
	
	//�������ݰ�
	packet_send(hp,0x02,3+CHANNEL_ID_MAX,data,system.addr485);
}
//���ع黹״̬
static void send_return_state(HPacket*hp,U8 s,U8 ch_addr)
{
	U8*data=hp->p.data;
	Channel*ch = channel_data_get_by_addr(ch_addr);
	if(ch==NULL){enable_485_rx();return;}
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
	packet_send(hp,0x06,2,send_buf,0xFF);//������	
}

/*===================================================
                �����������
====================================================*/
/*��������*/
static void com_send_tick(HPacket*hp)
{
	packet*p = &hp->p;
  U8*data = p->data;
	
	/*����*/
	int en_len = p->llen;
	if(en_len<=2)en_len=0;
	if(en_len>(2+CHANNEL_MAX))en_len=CHANNEL_MAX;
	
	/*������ʹ�ܷ�ʽ*/
	if(p->llen>0)system.enable = (BOOL)data[0]; //�����Ƿ�������
	if(p->llen>1)system.mode   = data[1];       //ǿ�� or ����
	if(en_len)memcpy(system.chs,data+2,en_len); //ǿ�Ʊ�־ 
	#ifndef NOT_USING_IR
	if(p->llen > (2+CHANNEL_MAX))system.iic_ir_mode = (SYSTEM_IIC_IR_MODE)data[2+CHANNEL_MAX];//����,iicģʽѡ�� :1ֻiic  2:ֻir  3:����iic
	#endif
	
	/*����������*/
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
static volatile U8 lch=0;                      							 //�ֵ�λ��
static volatile U32 ltimeout=0;                							 //��ʱʱ��	
BOOL is_system_lease(void) {return (BOOL)( (lch!=0) && (ltimeout!=0));} //�Ƿ����
AUTOSTART_THREAD_WITHOUT_TIMEOUT(comm_lease)
{
	static HPacket*hp;
	static Channel*pch;
	static U8 buffer[20];
//	static int bcounter = 0;  //��ŷ�����ʱ500,�ߵ�ƽ����
	static int i = 0;
	
	PROCESS_BEGIN();
	while(1){
	  PROCESS_WAIT_EVENT();//�ȴ��¼�����
		if(ev== PROCESS_EVENT_COMM_LEASE && data !=NULL)
		{
			hp = (HPacket*)data;                  
			pch = channel_data_get_by_addr(hp->p.data[0]);
			memcpy(buffer,hp->p.data,20);
			send_ack(hp,0x02,0x01);//����Ӧ��
			system.state = SYSTEM_STATE_LEASE;//ϵͳ״̬:���
			lch = buffer[0];                  //��������:���λ��
			ltimeout = 1000*buffer[11];      //��������:���ʱ��(ms)

			//��籦��Ų��ԣ���籦���Ϊ0==>differ
			if( (pch==NULL) ||  (buffer_cmp(pch->id,buffer+1,CHANNEL_ID_MAX)==FALSE) || (channel_id_is_not_null(pch->id)==FALSE) ){
				
				#ifdef USING_DEBUG_INFO
				/*pch==NULL:7  !=:6  ==NULL:5 */
				  if(pch==NULL) ld_debug_printf(7,lch,0,0);
				  if(buffer_cmp(pch->id,buffer+1,CHANNEL_ID_MAX)==FALSE) ld_debug_printf(6,lch,0,pch->iic_ir_mode);
				  if(channel_id_is_not_null(pch->id)==FALSE) ld_debug_printf(5,lch,0,pch->iic_ir_mode);
				#endif
				
				send_lease_state(hp,Lease_differ,lch,((pch==NULL)?(buffer+1):(pch->id)));
				goto LEASE_RESET_CONTINUE;
			}//��籦��Ų���
			
			//if(pch->state.read_ok)//��籦�Ƿ���Ч
			{
				if(!is_ver_5())//��5����������
				{
					int result=0;
					U8 lock[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
					//���Խ������Σ�������æʱ����ȴ�ʱ��2.5��
					for(i=0;i<UNLOCK_RETRY_TIMES;i++)
					{
						result = channel_read(pch,RC_UNLOCK,lock,650,(((i+1)==UNLOCK_RETRY_TIMES)?TRUE:FALSE)); //���һ�ο���תͨѶ��ʽ
						if( (result!=TRUE) || (lock[0] != 0x05))
							continue;
						else 
							break;
					}
					if( (result!=TRUE) || (lock[0] != 0x05) ){//����ʧ��
						send_lease_state(hp,Lease_decrypt_fall,lch,buffer+1);
						goto LEASE_RESET_CONTINUE;
					}
				}	

				if(diancifa(pch,ltimeout/1000,500,60,20)==FALSE)//��ŷ�����ʱ��500ms,����ʱ�� ltimout/1000��,�ڱۿ���(�ߵ�ƽʱ��<250ms)
				{
					if(channel_check_from_iic(pch))
					{
						send_lease_state(hp,Lease_dianchifa_fall,lch,buffer+1);//��ŷ�ʧ��
						pch->error.motor = 1;//��ŷ�����			
					  goto LEASE_RESET_CONTINUE;
					}
				}
				//��ŷ��ɹ�
				{
					request_charge_off(channel_data_get_index(pch)); delayms(10);//����ڳ�磬���Ϲص�
					pch->error.motor = 0;//��ŷ�������0
					send_lease_state(hp,Lease_success,lch,buffer+1); //�ɹ�===>Ӧ���
				  channel_data_clear_by_addr(lch);                 //�ɹ�===>������
				}
			}
			
			LEASE_RESET_CONTINUE:
			lch=ltimeout=0;
		}
	}
	PROCESS_END();
}

/*�黹����*/
static volatile U8 rch=0;                      							//�ֵ�λ��
static volatile U32 rtimeout=0;                							//��ʱʱ��	
BOOL is_system_in_return(U8 addr){return (BOOL)((rch!=0)&& (rtimeout!=0) && (rch==addr));}
static void com_return(HPacket*hp)
{
	packet *p = &hp->p;
	Channel*ch = channel_data_get_by_addr(p->data[0]);//�ֵ���ַ
  //��������
	system.state = SYSTEM_STATE_RETURN;//ϵͳ״̬:�黹//�����黹���첽����
	rch = p->data[0];                   //��������:�黹λ��
	rtimeout = 1000*p->data[1]+time(0); //��������:�黹��ʱ(ms)
	channel_led_flash(channel_data_get_index(ch),p->data[1]);//����
	send_ack(hp,0x03,0x01);         //����Ӧ��
}

/*��������*/
AUTOSTART_THREAD_WITHOUT_TIMEOUT(comm_ctrl)
{
	static U8 dataout[16];
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
				
			cmd=hp->p.data[0];
			memcpy(bao_id,&hp->p.data[2],10);//��籦���
			ctrl_time=hp->p.data[12];        //����ʱ��
			if(hp->p.llen<13)continue;
			send_ack(hp,0x04,0x01);          //����Ӧ��
			if(cmd==0x00)//�豸����
			{
				send_ctrl_state(hp,cmd,Cmd_success);
				delayms(1000);
				cpu_nvic_reset();
				continue;
			}
			//cmd =01 02 �Բֵ���������ȡ�ֵ�����
			pch = channel_data_get_by_addr(ch_addr);if(pch==NULL)continue;	
			if(cmd==0x01)//��ά�򿪲��ţ�ʹ�ܳ��1Сʱ
			{
							
				if(!(data_is_ver_5(bao_id[6])))//6�������ϣ�����1Сʱ
				{
					 channel_read(pch,RC_UNLOCK_1HOUR,dataout,650,TRUE);
				}
			}
			
			if(cmd==0x01||cmd==0x02)//ǿ�ƿ���	
			{			

				if(diancifa(pch,ctrl_time,500,60,20))//��ŷ�����ʱ��500ms,����ʱ��ctrl_time��,�ڱۿ���(�ߵ�ƽʱ��<250ms)
				{
					request_charge_off(channel_data_get_index(pch)); //����ڳ�磬���Ϲص�
					pch->error.motor = 0;                            //��ŷ�������0
					send_ctrl_state(hp,cmd,Cmd_success );            //�ɹ�===>����Ӧ��
					 channel_data_clear_by_addr(ch_addr);            //�ɹ�===>������
				}
				else
				{
					pch->error.motor = 1;            //��ŷ�����
					send_ctrl_state(hp,cmd,Cmd_fall);//��ŷ�ʧ��
				}
			}
		}
    memset(hp,0,sizeof(HPacket));		
		continue;
	 }
		
	PROCESS_END()
}
	
/*����̼�����*/
AUTOSTART_THREAD_WITHOUT_TIMEOUT(comm_entry)
{
	static HPacket*hp;
	PROCESS_BEGIN();
	while(1)
	{
		PROCESS_WAIT_EVENT();
		if(ev== PROCESS_EVENT_COMM_ENTRY && data !=NULL)
		{
			hp = (HPacket*)data;
			//�����־ 
			system.firmware_updata_flag=0x88;
			ld_flash_write(0,&system.firmware_updata_flag,1,FIRMWARE_updata);
			send_update_state(hp,0x01);
			delayms(1000);
			cpu_nvic_reset();//��λ		
		}
	}
	PROCESS_END();
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

/*��չ��Ϣ:��������*/
#ifdef USING_DEBUG_INFO
static void com_debug_info(HPacket*hp)
{
	/*data[0] = 01 ��������Ϣ   [1] offset [2]counter
	            02��Ϣ��0*/
	/*���� 03 ��+��Ϣ           [04] ��Ϣ��ɹ� 0 or 1*/
	if(hp->p.data[0]==01)
	{
		int l = ld_debug_counter();
		int o=hp->p.data[1];
		int c=hp->p.data[2];
		while(o<l&&c>0)
		{
			hp->p.data[0] = 3;	
			ld_debug_read(o,(char*)&hp->p.data[1]);
			packet_send(hp,PC_DEBUGINFO,1+sizeof(DebugInfo),hp->p.data,system.addr485);	
			delayms(20);
			o++;
			c--;			
		}
	}
	else if(hp->p.data[0]==02)
	{
		ld_debug_printf_clear();
		hp->p.data[0] = 4;
		hp->p.data[1] = 1;
		packet_send(hp,PC_DEBUGINFO,2,hp->p.data,system.addr485);
	}
}
#endif

/*������䴦����*/
static void com_process(HPacket*hp)
{
	packet *p = &hp->p;
	
	//��ַ����
	if(p->addr==system.addr485)
		switch(p->cmd)
		{
			case PC_HEART_BREAK	:   ld_system_flash_led(500,5);	com_send_tick(hp);		return;                   //ϵͳ��500ms������5��,��������
			case PC_LEASE				:		process_post(&thread_comm_lease,PROCESS_EVENT_COMM_LEASE,(void*)hp);return; //�¼����͸����߳�thread_lease
			case PC_RETURN			:		com_return(hp);				return;                                               //��¼һ�£�Ȼ���� return �߳�����ɹ黹����
			case PC_CTRL				:		process_post(&thread_comm_ctrl,PROCESS_EVENT_COMM_CTRL,(void*)hp);return;   //ͬ������,�¼����͸����߳�thread_lease
			case PC_UPDATE_ENTRY:		process_post(&thread_comm_entry,PROCESS_EVENT_COMM_ENTRY,(void*)hp);return;   //ͬ������,�¼����͸����߳�thread_entry                                               //����������־����λ==>bootloader�������
			case PC_UPDATE_MODE :		com_update_mode_query(hp);return;
			                                                
			case PC_UPDATE_DATA://���ݷ���bootloader�������ﲻ����
				
			#ifdef USING_DEBUG_INFO
			case PC_DEBUGINFO   :   com_debug_info(hp);return;  //�������� 
			#endif
			
			default:break;

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
			return;
			case 0x016: send_handshake_state(hp,0x01); return;//����
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
	ld_uart_open(COM_485,115200,8,0,1,0,400);  //�򿪴���
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
                �첽�黹����
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(return)
{
	U8 dataout[16];
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
						static int result = 0;
						static int i=0;
						
						//�첽�黹ʱ��������
						for(i=0;i<RETURN_READ_TIMES;i++)
						{
																result = channel_read(pch,RC_READ_ID,dataout,550,TRUE);
							if(result==FALSE) result = channel_read(pch,RC_READ_ID,dataout,550,TRUE);    
							if(result==TRUE)  result = channel_read(pch,RC_READ_DATA,dataout,650,TRUE);
							if(result==TRUE){pch->state.read_ok=1;break;}//�ɹ���ȡ����һʱ���־
						}

						if(!channel_id_is_not_null(pch->id) || (result!=TRUE) )//�޷�ʶ��
							send_return_state(&hpacket,Return_unrecognized,rch);
						else
							send_return_state(&hpacket,Return_success,rch);
					
						pch->flash=FALSE;//����˸
						system.state=SYSTEM_STATE_INIT;//��λ
						rch=0;
						rtimeout=0;
					}
				}
			}
		}else{rch=rtimeout=0;}
		os_delay(return,100);
		ld_iwdg_reload();
	}
	PROCESS_END();
}


