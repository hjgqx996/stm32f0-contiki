#include "includes.h"


/*===================================================
               ��
====================================================*/

#define isvalid_daowe()  ld_gpio_get(ch->map->io_detect) //��λ������Ч
#define isvalid_baibi()  ld_gpio_get(ch->map->io_sw)     //�ڱۿ�����Ч
#define is_ver_6()       ((ch->id[6]&0x0F)==0x06)        //6����
#define is_ver_7()       ((ch->id[6]&0x0F)==0x07)        //7����
#define is_ver_lte_5()   ((ch->id[6]&0x0F)==0x05)        //5��������
#define is_output()      ((ch->


//�ȴ�����
#define channel_read_wait()         do{while(channel_read_busy(i,mode)){os_delay(channel,10);}}while(0) 	
//��һ���ֵ���һ������,xΪ�ɹ���Ҫִ�еĴ���
#define channel_read_start_wait_end(cmd,x)   \
				/*��ʼ����,����ʱ�Ǵ�*/   if(!channel_read_start(i,mode,ch->iic_dir,cmd)){mode=iic_ir_select_poll(ch,TRUE);} \
				/*�ȴ����*/							else{ while(channel_read_busy(i,mode)){os_delay(channel,10);} \
				/*�жϽ��*/              if(channel_read_end(i,mode,buf)!=2){mode=iic_ir_select_poll(ch,TRUE);} \
				/*�����ȷʱ����x*/       else{x;}}
	
/*===================================================
                ˽�к���
====================================================*/

//����������ж�ѡ�����ַ�ʽ��ͨѶ==>�����Ǵ������ص�ǰѡ���ͨѶ��ʽ
static READ_TYPE_MODE iic_ir_select_poll(Channel*ch,BOOL error)
{
	READ_TYPE_MODE mode;
  if(system.iic_ir_mode==SIIM_ONLY_IR) {return RTM_IR;}	//ǿ��ʹ�� ir		
	if(system.iic_ir_mode==SIIM_ONLY_IIC){mode = RTM_IIC;goto READ_TYPE_MODE_IIC;}//ǿ��ʹ�� iic	
	
	READ_TYPE_MODE_IIC_IR://��ʽ�л�
  mode = (ch->iic_ir_mode);
	if(ch->iic_ir_mode_counter>=IIC_IR_SWITCH_ERROR_MAX)//�л�
	{
		ch->iic_ir_mode=0x01&(!ch->iic_ir_mode);
		ch->iic_dir_counter=ch->iic_ir_mode_counter=0;
		mode = ch->iic_ir_mode;
		return mode;
	}
	if(mode==RTM_IR)
	{
		ch->iic_ir_mode_counter+=(error==TRUE)?1:0;//�л������ۼ�
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
/*��������*///[0] �汾�� [1] ���� [2] �¶� [3] ������ [4-5] ѭ������ [6-7] ���� [8-9] ��о��ѹ [10-11] ���� (��λ��ǰ)
static void save_data(Channel*ch,U8*data)
{
	ch->Ver 							= data[0];
	ch->Ufsoc							=	data[1];
	ch->Temperature				=	data[2];
	ch->CycleCount				= (((U16)data[5])<<8)|(data[4]);
	ch->RemainingCapacity	= (((U16)data[7])<<8)|(data[6]);
	ch->Voltage          	= (((U16)data[9])<<8)|(data[8]);
	ch->AverageCurrent    = (((U16)data[11])<<8)|(data[10]);
}

/*===================================================
               �ֵ�����
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(channel)
{
	static U8 i = 0;
	static Channel *ch;
	static READ_TYPE_MODE mode;
	static U8 buf[13];
	PROCESS_BEGIN();
  channel_data_init();//��ʼ���ֵ�����
	while(1)
	{
		/*--------------------------------------------------------
		1.������ͨ����飬����籦�Ƿ���Ч
		2.��Ч�ĳ�籦ִ�����²���:
		  (1) ��ID 
		  (2) ������
		  (3) �������־
		  (4) 6��������,����������£�Ӧ�� ����
		3.ִ��Ƶ�� 2.8��
		---------------------------------------------------------*/	
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			ch = channel_data_get(i);
			if(ch==NULL)continue;
			
			if(isvalid_baibi() && isvalid_daowe() )															//�жϳ�籦�Ƿ���Ч
			{		
				mode = iic_ir_select_poll(ch,FALSE);  														//ѡ��iic����ir��ͨѶ
        if(channel_read_busy(i,mode))continue;														//æ����һ���ֵ�
				
				channel_read_start_wait_end(RC_READ_ID,memcpy(ch->id,buf,10));		//��ȡID
				
				channel_read_start_wait_end(RC_READ_DATA, save_data(ch,buf));			//��ȡ����
				
				if(mode==RTM_IIC)
					{channel_read_start_wait_end(RC_OUTPUT,ch->bao_output=buf[0]);}	//���ʹ��iic,Ҫ��ȡ�����־
				
				//�����������(6�����ϣ���ֹ���,7������ʹ�ú���ͨѶ��ֹ���)
			}
		}
    os_delay(channel,2800);
	}
	PROCESS_END();
}
