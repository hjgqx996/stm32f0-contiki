
/*===================================================
����:�ֵ�<====(iic,ir����)=====>��籦
1.���԰���:
  (1)���ָ��iic,��ֻ��ʹ��iicͨѶ
	(2)���ָ��ir,��ֻ��ʹ��irͨѶ
	(3)���߶�����ͨѶʱ�������κ��л�
	(4)iic���γ����κ��л���������ٴ��Σ���Ӧ���л���ir
2.ʵ�ַ�ʽ
  ״̬��: �����룬������
3.���ܰ���:
  (1)��id
  (2)������
  (3)����������������1Сʱ,��������־	
4.�������ʹ�ýӿ�:
  while(channel_read(ch_index,cmd,dataout,timeout)==FALSE){ osdelay(...)}
  �ȴ���ʱ��cpu���л�������Ӱ���������������	
====================================================*/
#include "includes.h"

/*===================================================
                ���غ���
====================================================*/
/*ѡ�����*/
static READ_TYPE_MODE iic_ir_select_poll(Channel*ch,BOOL error,BOOL clear)
{
	READ_TYPE_MODE mode;
	
	if(clear==TRUE)
	{
		ch->iic_dir_counter=0;
		ch->iic_ir_mode_counter=0;
	}

  if(system.iic_ir_mode==SIIM_ONLY_IR) {return RTM_IR;}	//ǿ��ʹ�� ir		
	if(system.iic_ir_mode==SIIM_ONLY_IIC){mode = RTM_IIC;goto MODE_IIC_POLL;}//ǿ��ʹ�� iic	
	
	if(system.iic_ir_mode==SIIM_IIC_IR)
	{
		mode = (ch->iic_ir_mode);
		if(mode==RTM_IR)        //����
		{
		  ch->iic_ir_mode_counter+=(error==TRUE)?1:0;
			if(ch->iic_ir_mode_counter>=IR_TO_IIC_SWITCH_ERROR_MAX)//�л������ۼ�
			{
				ch->iic_ir_mode_counter=0;
				ch->iic_dir_counter=0;
				ch->iic_ir_mode=RTM_IIC;
			}
		}
		else if(mode==RTM_IIC) //IIC
		{
			if(ch->iic_ir_mode_counter>=IIC_TO_IR_SWITCH_ERROR_MAX*2)//�л������ۼ�
			{
				ch->iic_ir_mode_counter=0;
				ch->iic_dir_counter=0;
				ch->iic_ir_mode=RTM_IR;
			}
			MODE_IIC_POLL:
			ch->iic_dir_counter+=(error==TRUE)?1:0;
			if(ch->iic_dir_counter>=IIC_DIR_SWITCH_MAX)
			{
				ch->iic_dir_counter=0;
				ch->iic_dir=(ch->iic_dir+1)&0x01;
				ch->iic_ir_mode_counter++;
			}
		}
	}
	
	return ch->iic_ir_mode;
}
/*----------------------------------
��籦�����ض���:�Ƿ�æ,�� ,�Ƿ����
-----------------------------------*/
static BOOL channel_read_busy(Channel*pch)
{
	U8 ch = channel_data_get_index(pch);
	if(pch->iic_ir_mode == RTM_IIC)return ld_iic_busy(ch);
	if(pch->iic_ir_mode == RTM_IR )return ld_ir_busy (ch);
	return FALSE;
}
static BOOL channel_read_cmd(Channel*pch,U8 cmd)
{
	U8 ch = channel_data_get_index(pch);
	if(pch->iic_ir_mode == RTM_IIC)return ld_iic_cmd(ch,cmd);
	if(pch->iic_ir_mode == RTM_IR )return ld_ir_cmd(ch,cmd);
	return FALSE;
}

static BOOL channel_read_start(Channel*pch,READ_TYPE_CMD cmd)
{
	U8 wanlen=0;
	U8 ch = channel_data_get_index(pch);
	if(pch->iic_ir_mode == RTM_IIC)
	{
		switch(cmd)
		{
			case RC_READ_ID:wanlen=10;break;
			case RC_READ_DATA:wanlen=13;break;
			case RC_OUTPUT:wanlen=1;break;
			case RC_LOCK:case RC_UNLOCK: case RC_UNLOCK_1HOUR: wanlen=0;break;
			default: return FALSE;
		}
		return ld_iic_read_start(ch,pch->iic_dir, cmd,wanlen);
	}
	else if(pch->iic_ir_mode ==RTM_IR){
		switch(cmd)
		{
			case RC_READ_ID:wanlen=7;break;
			case RC_READ_DATA:wanlen=13;break;
			case RC_OUTPUT:return FALSE;
			case RC_LOCK:case RC_UNLOCK: case RC_UNLOCK_1HOUR: wanlen=2;break;
			default: return FALSE;
		}
		return ld_ir_read_start(ch,pch->iic_dir, cmd,wanlen);
	}
	else return FALSE;
}

static int channel_read_end(Channel*pch,U8*dataout)
{
	U8 wanlen=0;
	U8 ch = channel_data_get_index(pch);
	if(pch->iic_ir_mode == RTM_IIC)
	{
		return ld_iic_read_isok(ch,dataout,0);
	}
	else if(pch->iic_ir_mode ==RTM_IR){
		return ld_ir_read_isok(ch,dataout,0);
	}
	else return 2;
}

/*===================================================
                ��籦��״̬�� 
����:  
state: ��ǰ״̬[input]
   ��ʼֵ=0;
0:������δ��ʼ
1:������������ 
2:������ɹ�  
3:�������޷���ȡ
4:��ʱ�Ǵ�һ��
====================================================*/
U8 channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout)
{
	#define s (pch->cmd_state)
	U8 ch = channel_data_get_index(pch);
	
	//����δ����
	if(s==0)
	{
		//�Ǳ�����������
		if(channel_read_busy(pch)) return s;
		else
		{
			if(channel_read_start(pch,cmd)==FALSE)
			{
				goto CHANNEL_READ_ERROR;
			}
			else {pch->thread=PROCESS_CURRENT();s =1;return 1;}
		}
	}
	//�������Ѿ���ʼ
	if(s==1)
	{
		if( channel_read_busy(pch) && channel_read_cmd(pch,cmd) && (pch->thread==PROCESS_CURRENT())){return 1;}
		{
			int err= channel_read_end(pch,dataout);
		  if(err<0)//������ʧ��
			{
				goto CHANNEL_READ_ERROR;
			}          
		  else if(err==2)//������ɹ�
			{
				s=0;
				iic_ir_select_poll(pch,FALSE,TRUE);
				return 2;
			} 
	  }
	}
	
	if(s>=2)
	{
		//��ʱ
		if(s==4){			
			CHANNEL_READ_ERROR:
				iic_ir_select_poll(pch,TRUE,FALSE);//�Ǵ�һ��
			  pch->thread=0;
		}
		s=0;
    return 3;	
	}
}