
/*===================================================
����:��ֵ�������غ���
====================================================*/
#include "includes.h"
#include "stm32f0xx.h"

/*===================================================
                �꣬����
====================================================*/
/*�ֵ���籦����Ƿ�ΪNULL*/
static const U8 null_id[10] = {0,0,0,0,0,0,0,0,0,0};
BOOL channel_id_is_not_null(U8*id){return (buffer_cmp((U8*)null_id,id,CHANNEL_ID_MAX)==TRUE)?FALSE:TRUE;}

/*���еĲֵ����ݻ���*/
static Channel chs[CHANNEL_MAX]={0};



/*===================================================
                ���غ���
====================================================*/
/*-------------------------------------
�ֵ�����״̬
-------------------------------------*/
static void channel_state_check(U8 ch)
{
  Channel*pch = channel_data_get(ch);if(pch==NULL)return;
	
	/*�б�,��ȡ����*/
	if(isvalid_daowe() && isvalid_baibi() && is_readok() && channel_id_is_not_null(pch->id))
	{
		pch->state.read_ok=1;
		pch->state.read_error=0;
	}else pch->state.read_ok=0;
	
	/*�б�,��ȡ������*/
	if( (isvalid_daowe()) && ( (is_readerr()) /*|| (channel_id_is_not_null(pch->id)==FALSE)*/ ) )
	{
		pch->state.read_error=1;
		pch->state.read_ok=0;
	}else pch->state.read_error=0;
	/*���ڳ��*/
	pch->state.charging = isin5v();
	/*-----������--:�ڳ����������--*/
	
	/*-----iic or ir------------------*/
	if(isvalid_baibi())
		pch->state.read_from_ir = pch->iic_ir_mode;
	else 
		pch->state.read_from_ir = 0;
}
/*-------------------------------------
�ֵ��澯
-------------------------------------*/
static void channel_warn_check(U8 ch)
{
	U8 d = 0;
    #define out5v()  ld_gpio_get(pch->map->io_sw)
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	/*�¶ȸ澯:<0 or >60*/
	if( (pch->Temperature>BAO_WARN_TEMPERATURE) || (pch->Temperature<0) )pch->warn.temperature=1;
	else pch->warn.temperature=0;
	
	/*����:���¼�����*/
	
	/*5V���澯��5v���������ƽ��ͬ*/
	d = 0;
	if(isin5v() != isout5v())
	{
		delayus(50);
		if(isin5v() != isout5v())
		{
			delayus(50);		
			if(isin5v() != isout5v())
			{
			 d = pch->warn.mp=1;
			}
		}
	}
  pch->warn.mp = d;			
}
/*-------------------------------------
�ֵ�����
-------------------------------------*/
static void channel_error_check(U8 ch)
{
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	//��λ����
	if( (isvalid_daowe()==0) && (pch->state.read_ok==1) )pch->error.daowei=1;
	else pch->error.daowei=0;
	
	//�ڱۿ��ع���
	if(  			( (isvalid_baibi()==0)&&(isvalid_daowe()==1))                            //�ڱ��ޣ���λ��
			|| 		( (isvalid_baibi()==1)&&(isvalid_daowe()==0)&&(pch->state.read_ok==0) )  //�ڱ��У���λ�ޣ���������
	  )
	{
		pch->error.baibi=1;
	}else pch->error.baibi=0;
	
	//�¶� <0 or >65:���籦����0x20
	if( (pch->Temperature<0) || (pch->Temperature>BAO_ERROR_TEMPERATURE) )pch->error.temp=1;
	else pch->error.temp=0;
	
	//����ʶ�����,ʶ�����>=2,���Ұڱۿ��طǹ�����
	if( (pch->iic_error_counter>=BAO_DINGZHEN_ERROR_TIMES) && (pch->error.baibi==0) )
		pch->error.thimble=1; 
	else pch->error.thimble = 0;
	
	//����ʶ�����
	if( (pch->ir_error_counter>=BAO_IR_ERROR_TIMES)  && (pch->error.baibi==0))
		pch->error.ir=1;
	else 
	{
		pch->error.ir=0;
	}
	
	//��ŷ�����===>״̬�ڵ�ŷ�����ʱ ���жϣ����ڴ��ж�
	
	//�豦����  ===>���������ظ����� ���жϣ����ڴ��ж�
	//�豦ָ��� ���ɹ�������Ϊ�� �豦����
	
	/*---------------���ع�������-----------------------------------*/
	if(pch->error.baibi || pch->error.daowei) 
		ld_system_flash_led(100,2); //���ش���100ms,��2��
}
/*===================================================
                ȫ�ֺ���
====================================================*/

/*--------------------channel���ݳ�ʼ��-------------*/
BOOL channel_data_init(void)
{
	int i =0;
	memset(chs,0,sizeof(chs));
	for(;i<CHANNEL_MAX;i++)
	{
		chs[i].map = &channel_config_map[i]; //io
		ld_ir_init(i+1,channel_config_map[i].io_ir,channel_config_map[i].io_re);
	}	
	return TRUE;
}


/*--------------- �����ch(1-n)���ֵ�������----------- */
BOOL channel_data_clear(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	if(pch==NULL)return FALSE;
	memset((void*)&(pch->Ufsoc),0,sizeof(Channel)-((int)&(pch->Ufsoc) - (int)pch));//����ַ�⣬������0
	memset((void*)&(pch->state),0,1);                  //״̬λ��
	pch->warn.temperature=0;                           //�¶ȱ�����
	pch->error.ir=pch->error.temp=pch->error.thimble=0;//����״̬��
	pch->iic_error_counter = pch->ir_error_counter = 0;//���룬������������
	pch->readerr = pch->readok = 0;
	return TRUE;
}

/*---------------- �����ַΪch_addr������---------- */
BOOL channel_data_clear_by_addr(U8 ch_addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==ch_addr)
		{
			memset((void*)&(chs[i].first_insert),0,sizeof(Channel)-((int)&(chs[i].first_insert) - (int)(&chs[i])));//����ַ�⣬������0
		}
	}
	return FALSE;
}

/*-------------- ��ȡ�ֵ����� channel:1-n----------------*/
Channel*channel_data_get(U8 channel)
{
	channel-=1;
	if(channel>=CHANNEL_MAX)return NULL;
	return &chs[channel];
}
/*--------------- ��ȡ�ֵ�����--by addr-------------------*/
Channel*channel_data_get_by_addr(U8 addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==addr)
			return &chs[i];
	}
	return NULL;
}
/*------------��ȡ�ֵ�����,error:return <0: return :1-n-----*/
int channel_data_get_index(Channel*ch)
{
	U32 coffset = 0;
	U8 index = 0;
	coffset = (U32)ch - (U32)chs;
	if(coffset%sizeof(Channel)==0)index=coffset/sizeof(Channel);
	else return -1;
	if(index>=CHANNEL_MAX)return -1;
	return index+1;
}
/*-------------���òֵ���ַ----------------------------------*/
void channel_addr_set(U8*addrs)
{
	int i=0;Channel *vch;
	for(i=0;i<=CHANNEL_MAX;i++)
	{
		vch=channel_data_get(i+1);if(vch==NULL)continue;
		vch->addr=addrs[i];//����ͨ����ַ
	}
}

/*-------------��������----------------------------------*/
//[0] �汾�� [1] ���� [2] �¶� [3] ������ [4-5] ѭ������ [6-7] ���� [8-9] ��о��ѹ [10-11] ���� (��λ��ǰ)
void channel_save_data(Channel*ch,U8*data)
{
	if(ch==NULL||data==NULL)return;
	ch->Ver					= data[0];
	ch->Ufsoc				= data[1];
	ch->Temperature			= data[2];
	ch->CycleCount			= (((U16)data[5])<<8)|(data[4]);
	ch->RemainingCapacity	= (((U16)data[7])<<8)|(data[6]);
	ch->Voltage				= (((U16)data[9])<<8)|(data[8]);
	ch->AverageCurrent		= (((U16)data[11])<<8)|(data[10]);
}
/*-------------�ֵ�״̬1���һ��----------------------------------*/
void channel_check_timer_1s(void)
{
	int i=1;
	for(;i<=CHANNEL_MAX;i++)
	{
		channel_state_check(i);
		channel_warn_check(i);
		channel_error_check(i);	
	}		
}

/*----------------------------------
   �ֵ���
ch:�ֵ���   seconds:��˸ʱ��  timer_ms:������ʱ��
���ʱ���ֵ�����˸Ƶ��0.5s
-----------------------------------*/
void channel_led_flash(U8 ch,U8 seconds)
{
  Channel*channel = channel_data_get(ch);
	if(channel==NULL)return;
	channel->flash=TRUE;
	channel->flash_ms=1000*seconds;
	channel->flash_now=0;
}
void channels_les_flash_timer(int timer_ms)
{
	//��ѯ����ͨ��,����ֵ���
	int i=0;
	for(;i<CHANNEL_MAX;i++)
	{
		Channel*ch =channel_data_get(i+1);
		if(ch->map==NULL)continue;
		if(ch->flash){
			ch->flash_now+=timer_ms;
			if(ch->flash_now>=LEASE_LED_FLASH_TIME)
			{	
				//ʱ��ݼ�
				ch->flash_now-=LEASE_LED_FLASH_TIME;
				ch->flash_ms-=LEASE_LED_FLASH_TIME;
				if(ch->flash_ms<LEASE_LED_FLASH_TIME)
				{
					ch->flash=FALSE;
					ld_gpio_set(ch->map->io_led,LOW);//ֹͣ��˸
				}
				else
					ld_gpio_set(ch->map->io_led,ld_gpio_get(ch->map->io_led)==0?HIGH:LOW);
			}
		}
		else{
			if( (ch->Ufsoc>CHANNEL_LED_LIGHT_UFSOC) && (ch->state.read_ok) )//��������50%����
				ld_gpio_set(ch->map->io_led,HIGH);
			else 
				ld_gpio_set(ch->map->io_led,LOW);
		}
	}
}
