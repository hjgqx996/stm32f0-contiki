#include "includes.h"

//��λ������Ч
#define isvalid_daowe()  ld_gpio_get(ch->map->io_detect) 
//�ڱۿ�����Ч
#define isvalid_baibi()  ld_gpio_get(ch->map->io_sw) 

//����������ж�ѡ�����ַ�ʽ��ͨѶ
static READ_TYPE_MODE iic_ir_select_poll(Channel*ch,BOOL error)
{
	READ_TYPE_MODE mode;
  if(system.iic_ir_mode==SIIM_ONLY_IR) {return RTM_IR;}	//ǿ��ʹ�� ir		
	if(system.iic_ir_mode==SIIM_ONLY_IIC){mode = RTM_IIC;goto READ_TYPE_MODE_IIC;}//ǿ��ʹ�� iic	
	
	READ_TYPE_MODE_IIC_IR://��ʽ�л�
  mode = ch->iic_ir_mode;
	if(mode==RTM_IR)
		ch->iic_ir_mode_counter+=(error==TRUE)?1:0;//�л������ۼ�
	
	if(ch->iic_ir_mode_counter>=IIC_IR_SWITCH_ERROR_MAX)//�л�
	{
		ch->iic_ir_mode=0x01&(!ch->iic_ir_mode);
		ch->iic_dir_counter=ch->iic_ir_mode_counter=0;
		mode = ch->iic_ir_mode;
		return mode;
	}
	
	READ_TYPE_MODE_IIC://�����л�
	ch->iic_dir_counter+=(error==TRUE)?1:0;//�����ۼ�
	if(ch->iic_dir_counter>=IIC_DIR_SWITCH_MAX)
	{
		ch->iic_dir=(!ch->iic_dir)&0x01;
		ch->iic_dir_counter=0;	
		ch->iic_ir_mode_counter++;//�л������ۼ�
	}
	return mode;
}
/*��������*/
static void save_data(Channel*ch,U8*data)
{

}

/*===================================================
                �ֵ�����
====================================================*/
static struct etimer et_channel;
PROCESS(thread_channel, "ͨ������");
AUTOSTART_PROCESSES(thread_channel);
PROCESS_THREAD(thread_channel, ev, data)  
{
	static U8 i = 0;
	static Channel *ch;
	static READ_TYPE_MODE mode;
	static U8 buf[13];
	PROCESS_BEGIN();
  channel_data_init();//��ʼ���ֵ�����
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++){
			ch = channel_data_get(i);
			if(ch==NULL)continue;
			
			if(isvalid_baibi() && isvalid_daowe() )//�жϳ�籦�Ƿ���Ч
			{		
				mode = iic_ir_select_poll(ch,FALSE);
        if(channel_read_busy(i,mode))continue;//æ����һ���ֵ�
				
				//��ȡID
				if(!channel_read_start(i,mode,ch->iic_dir,RC_READ_ID))
					iic_ir_select_poll(ch,TRUE);//����һ��
				while(channel_read_busy(i,mode)){os_delay(et_channel,20);}//�ȴ�����
				while(channel_read_end(i,mode,buf)!=2){os_delay(et_channel,20);}//�ȴ����
				memcpy(ch->id,buf,10);
				
				//��ȡ����
				if(!channel_read_start(i,mode,ch->iic_dir,RC_READ_DATA))
				iic_ir_select_poll(ch,TRUE);//����һ��
				while(channel_read_busy(i,mode)){os_delay(et_channel,20);}//�ȴ�����
				while(channel_read_end(i,mode,buf)!=2){os_delay(et_channel,20);}//�ȴ����
				save_data(ch,buf);
				
				//6�����ϣ���ֹ���,7������ʹ�ú���ͨѶ
				
			}
			
		}
     os_delay(et_channel,2800);
	}

	PROCESS_END();
}
