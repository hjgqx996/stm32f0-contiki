
/*===================================================
����:��ֵ�������غ���
====================================================*/
#include "includes.h"

/*===================================================
                �����ļ�
====================================================*/

/*===================================================
                �꣬����
====================================================*/
/*�ֵ���籦����Ƿ�ΪNULL*/
const U8 null_id[10] = {0,0,0,0,0,0,0,0,0,0};
BOOL channel_id_is_not_null(U8*id){return buffer_cmp((U8*)null_id,id,CHANNEL_ID_MAX);}
static Channel chs[CHANNEL_MAX]={0};
/*===================================================
                ���غ���
====================================================*/


/*===================================================
                ȫ�ֺ���
====================================================*/
/*
* channel���ݳ�ʼ��
*/
BOOL channel_data_init(void)
{
	int i =0;
	memset(chs,0,sizeof(chs));
	for(;i<CHANNEL_MAX;i++)
	{
		chs[i].map = &channel_config_map[i]; //io
		//chs[i].addr = //addr �ֵ���ַ
		ld_ir_init(i+1,channel_config_map[i].io_ir,channel_config_map[i].io_re);
	}	
	ld_ir_timer_init();//��������
}
BOOL channel_data_clear_by_addr(U8 ch_addr)
{
	int i =0;
	for(;i<CHANNEL_MAX;i++)
	{
		if(chs[i].addr==ch_addr)
		{
			memset(&chs[i],0,sizeof(Channel));
			return TRUE;
		}
	}
	return FALSE;
}


/*��ȡ�ֵ�����
*channel:1-n
*/
Channel*channel_data_get(U8 channel)
{
	channel-=1;
	if(channel>=CHANNEL_MAX)return NULL;
	return &chs[channel];
}
/*��ȡ�ֵ�����--by addr
*channel:1-n
*/
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
/*----------------------------------
6��������:���� ����
-----------------------------------*/



/*----------------------------------
�ֵ�����״̬
�ֵ��澯
�ֵ�����
-----------------------------------*/
void channel_state_check(U8 ch)
{

}

void channel_warn_check(U8 ch)
{

}

void channel_error_check(U8 ch)
{

}

/*----------------------------------
�ֵ�������
�ֵ�����ϵ�
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
				if(ch->flash_ms<LEASE_LED_FLASH_TIME){
					ch->flash=FALSE;
					ld_gpio_set(ch->map->io_led,LOW);
				}//ֹͣ��˸
				else
				ld_gpio_set(ch->map->io_led,ld_gpio_get(ch->map->io_led)==0?HIGH:LOW);
			}
		}
		else{
			//��������50%����
			if(ch->Ufsoc>CHANNEL_LED_LIGHT_UFSOC && 1/*��籦��Ч()*/)
			{
				ld_gpio_set(ch->map->io_led,HIGH);
			}
		}
	}

}