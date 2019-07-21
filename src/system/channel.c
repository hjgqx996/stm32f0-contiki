
/*===================================================
����:��ֵ�������غ���
====================================================*/
#include "includes.h"
#include "stm32f0xx.h"

/*===================================================
                �꣬����
====================================================*/
/*�ֵ���籦����Ƿ�ΪNULL*/
const U8 null_id[10] = {0,0,0,0,0,0,0,0,0,0};
BOOL channel_id_is_not_null(U8*id){return !buffer_cmp((U8*)null_id,id,CHANNEL_ID_MAX);}

/*���еĲֵ����ݻ���*/
static Channel chs[CHANNEL_MAX]={0};

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
		ld_iic_init(i+1,channel_config_map[i].io_sda,channel_config_map[i].io_scl);
	}	
}

/*---------------- �����ַΪch_addr������---------- */
BOOL channel_data_clear_by_addr(U8 ch_addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==ch_addr)
		{
			memset((void*)&(chs[i].Ufsoc),0,sizeof(Channel)-((int)&(chs[i].Ufsoc) - (int)&chs[i]));//����ַ�⣬������0
			return TRUE;
		}
	}
	return FALSE;
}

/*--------------- �����ch(1-n)���ֵ�������----------- */
BOOL channel_data_clear(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	if(pch==NULL)return FALSE;
	memset((void*)&(pch->Ufsoc),0,sizeof(Channel)-((int)&(pch->Ufsoc) - (int)pch));//����ַ�⣬������0
	return TRUE;
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
	

/*----------------------------------
�ֵ�����״̬
�ֵ��澯
�ֵ�����
-----------------------------------*/
void channel_state_check(U8 ch)
{
  Channel*pch = channel_data_get(ch);if(pch==NULL)return;
	
	/*�б�,��ȡ����*/
	if(isvalid_daowe() && isvalid_baibi() && pch->readok>=2 && channel_id_is_not_null(pch->id))
	{
		pch->state.read_ok=1;
		pch->state.read_error=0;
	}
	
	/*�б�,��ȡ������*/
	if(isvalid_daowe() && (pch->readok<2 || channel_id_is_not_null(pch->id)==FALSE))
	{
		pch->state.read_error=1;
		pch->state.read_ok=0;
	}
	/*���ڳ��*/
	pch->state.charging = isin5v();
	/*-----������--:�ڳ����������--*/
	
	/*-----iic or ir------------------*/
	pch->state.read_from_ir = pch->iic_ir_mode;
}

void channel_warn_check(U8 ch)
{
    #define out5v()  ld_gpio_get(pch->map->io_sw)
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	/*�¶�*/
	if(pch->Temperature>BAO_WARN_TEMPERATURE)pch->warn.temperature=1;
	else pch->warn.temperature=0;
	
	/*����:���¼�����*/
	
	/*���澯*/
	if(isin5v() != isout5v())pch->warn.mp=1;
	else pch->warn.mp=0;
}

void channel_error_check(U8 ch)
{
	Channel*pch = channel_data_get(ch);if(pch==NULL)return; 
	
	//��λ����
	if( (isvalid_daowe()==0) && (pch->state.read_ok==1) )pch->error.daowei=1;
	else pch->error.daowei=0;
	
	//�ڱۿ��ع���
	if(  			( (isvalid_baibi()==0)&&(isvalid_daowe()==1)) 
			|| 		( (isvalid_baibi()==1)&&(isvalid_daowe()==0)&&(pch->state.read_ok==0) )
	  )
	{
		pch->error.baibi=1;
	}else pch->error.baibi=0;
	
	//�¶�<0
	if( (pch->Temperature<0) || (pch->Temperature>BAO_ERROR_TEMPERATURE) )pch->error.temp=1;
	else pch->error.temp=0;
	
}
//�����¼�
void channel_lock_event(U8 ch)
{
   

}



/*----------------------------------
�ֵ�������
�ֵ�����ϵ�
�������г��ms
-----------------------------------*/
void channel_charge(U8 ch)
{


}
void channel_discharge(U8 ch)
{
  //�ֵ��ϵ� 
	
	//���Ŷӵ��б���ɾ�����ֵ�
//	queue_delete(ch);
}
void channel_discharge_all(int ms)
{

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
			if(ch->Ufsoc>CHANNEL_LED_LIGHT_UFSOC && ch->state.read_ok)//��������50%����
				ld_gpio_set(ch->map->io_led,HIGH);
			else 
				ld_gpio_set(ch->map->io_led,LOW);
		}
	}
}
