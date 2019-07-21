
/*===================================================
����:�ֵ�<====(iic,ir����)=====>��籦
1.���԰���:
  (1)���ָ��iic,��ֻ��ʹ��iicͨѶ
	(2)���ָ��ir,��ֻ��ʹ��irͨѶ
	(3)���߶�����ͨѶʱ�������κ��л�
	(4)iic���γ����κ��л���������ٴ��Σ���Ӧ���л���ir

2.���ܰ���:
  (1)��id
  (2)������
  (3)����������������1Сʱ,��������־	
3.�������ʹ�ýӿ�:
  while(channel_read(ch_index,cmd,dataout,timeout)==FALSE){ osdelay(...)}
  �ȴ���ʱ��cpu���л�������Ӱ���������������	
====================================================*/
#include "includes.h"

/*===================================================
                ���ر���
====================================================*/
static U8 state[CHANNEL_MAX]={0};      //��������ʱ��״̬
static void*thread[CHANNEL_MAX]={0};   //��������ʱ���߳�,ֻ֧�ֵ��߳����У���֧�ֶ��̲߳���
/*===================================================
                ���غ���
====================================================*/
/* ѡ�����
*  ch: �ֵ�����
*  error:ͨ���Ƿ��������ʱΪTRUE  (����ʧ��ʱ���Ǵ�һ��)
*  clear:��λ����
*  ���� :��ǰͨѶ��ʽ(iic or ir)
*/
static READ_TYPE_MODE iic_ir_select_poll(Channel*ch,BOOL error,BOOL clear)
{
	READ_TYPE_MODE mode;
	
	if(clear==TRUE)//������0
	{
		ch->iic_dir_counter=0;
		ch->iic_ir_mode_counter=0;
	}

  if(system.iic_ir_mode==SIIM_ONLY_IR) {return RTM_IR;}	                   //ǿ��ʹ�� ir		
	if(system.iic_ir_mode==SIIM_ONLY_IIC){mode = RTM_IIC;goto MODE_IIC_POLL;}//ǿ��ʹ�� iic	
	
	if(system.iic_ir_mode==SIIM_IIC_IR)                                      //���߶�������
	{
		mode = (ch->iic_ir_mode);
		if(mode==RTM_IR)        //��ǰ:����
		{
		  ch->iic_ir_mode_counter+=(error==TRUE)?1:0;
			if(ch->iic_ir_mode_counter>=IR_TO_IIC_SWITCH_ERROR_MAX)//�л������ۼ�
			{
				ch->iic_ir_mode_counter=0;
				ch->iic_dir_counter=0;
				ch->iic_ir_mode=RTM_IIC;
			}
		}
		else if(mode==RTM_IIC) //��ǰ:IIC
		{
			if(ch->iic_ir_mode_counter>=IIC_TO_IR_SWITCH_ERROR_MAX*2)//�л������ۼ�
			{
				ch->iic_ir_mode_counter=0;
				ch->iic_dir_counter=0;
				ch->iic_ir_mode=RTM_IR;
			}
			
			MODE_IIC_POLL:      //IICCͨѶʱ��ʧ�ܺ���ı�
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
static BOOL channel_read_busy(U8 ch,READ_TYPE_MODE mode)//�Ƿ�æ
{
	if(mode == RTM_IIC)return ld_iic_busy(ch);
	if(mode == RTM_IR )return ld_ir_busy (ch);
	return FALSE;
}
static BOOL channel_read_cmd(U8 ch,U8 cmd,READ_TYPE_MODE mode)//��ǰ�����Ƿ�cmd
{
	if(mode == RTM_IIC)return ld_iic_cmd(ch,cmd);
	if(mode == RTM_IR )return ld_ir_cmd(ch,cmd);
	return FALSE;
}
/*ch:1-n*/
static BOOL channel_read_start(U8 ch,READ_TYPE_CMD cmd,READ_TYPE_MODE mode,U8 dir)//��ʼ����
{
	U8 wanlen=0;//Ҫ��ȡ�ĳ���
	if(mode == RTM_IIC)
	{
		switch(cmd)
		{
			case RC_READ_ID:wanlen=10;break;
			case RC_READ_DATA:wanlen=13;break;
			case RC_OUTPUT:wanlen=1;break;
			case RC_LOCK:case RC_UNLOCK: case RC_UNLOCK_1HOUR: wanlen=1;break;
			default: return FALSE;
		}
		return ld_iic_read_start(ch,dir, cmd,wanlen);
	}
	else if(mode ==RTM_IR){
		switch(cmd)
		{
			case RC_READ_ID:wanlen=7;break;
			case RC_READ_DATA:wanlen=13;break;
			case RC_OUTPUT:return FALSE;
			case RC_LOCK:case RC_UNLOCK: case RC_UNLOCK_1HOUR: wanlen=2;break;
			default: return FALSE;
		}
		return ld_ir_read_start(ch,dir, cmd,wanlen);
	}
	else return FALSE;
}
/* 
* ��ѯ�����Ƿ����н���
* return : <0��error
*        :  0: �޲���
*        :  1: ���ڶ�
*        :  2: ����ȷ
*/
static int channel_read_end(U8 ch,U8*dataout,READ_TYPE_MODE mode)
{
	if(mode == RTM_IIC)
		return ld_iic_read_isok(ch,dataout,0);
	else if(mode ==RTM_IR)
		return ld_ir_read_isok(ch,dataout,0);
	else return 2;
}

/*===================================================
            ȫ�ֺ���

��籦��==>��������ѯ��ʽ

pch:�ֵ�����
cmd:������
dataout:�ɹ��󷵻ص����� 

����:  
	0:������δ��ʼ
	1:������������ 
	2:������ɹ�  
	3:�������޷���ȡ
====================================================*/
U8 channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout)
{
	#define s (state[ch-1])
	#define t (thread[ch-1])
	U8 ch = channel_data_get_index(pch);    //������ 1 ��ʼ
	READ_TYPE_MODE mode = pch->iic_ir_mode; //iic irģʽ   
	U8 dir = pch->iic_dir;                  //����
	if(ch==0||pch==NULL)return 0;

	//�����߳�����ʹ��
	if(channel_read_busy(ch,mode)&& (t!=PROCESS_CURRENT()))return 0;
	
	//����δ����
	if(s==0)
	{
		//�Ǳ�����������
		if(channel_read_busy(ch,mode)) return s;
		else
		{
			if(channel_read_start(ch,cmd,mode,dir)==FALSE)
			{
				goto CHANNEL_READ_ERROR;
			}
			else {t=PROCESS_CURRENT();s =1;return 1;}
		}
	}
	//�������Ѿ���ʼ
	if(s==1)
	{
		if( channel_read_busy(ch,mode) && channel_read_cmd(ch,cmd,mode) && (t==PROCESS_CURRENT())){return 1;}//��������������
		{
			int err= channel_read_end(ch,dataout,mode);
		  if(err<0)//������ʧ��
			{
				goto CHANNEL_READ_ERROR;
			}          
		  else if(err==2)//������ɹ�
			{
				s=0;
				t=0;
				 (pch,FALSE,TRUE);//������0
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
			  t=0;
		}
		s=0;
		t=0;
    return 3;	
	}
}