
/*===================================================
����:�ֵ�<====(iic,ir����)=====>��籦
1.���԰���:
			(1)���ָ��iic,��ֻ��ʹ��iicͨѶ
			(2)���ָ��ir,��ֻ��ʹ��irͨѶ
			(3)���߶�����ͨѶʱ�������κ��л�
			(4)��ʹ��iicʱ:�����κ��л���������ٴ��Σ���Ӧ���л���ir

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
const unsigned char TESTDATA1[] =
{
0x2a,      0x06, 0x04,           0x14,
//ѭ������ /�¶� /ʣ������           /����
};

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
		mode = (READ_TYPE_MODE)(ch->iic_ir_mode);
		if(mode==RTM_IR)        //��ǰ:����
		{
			if(error==FALSE)
			{
				ch->iic_ir_mode_counter = 0;
			}
		  else 
				ch->iic_ir_mode_counter++;
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
			if(error==FALSE)
			{
				ch->iic_ir_mode_counter=0;
			  ch->iic_dir_counter=0;
			}
			else ch->iic_dir_counter+=(error==TRUE)?1:0;
			if(ch->iic_dir_counter>=IIC_DIR_SWITCH_MAX)
			{
				ch->iic_dir_counter=0;
				ch->iic_dir=(ch->iic_dir+1)&0x01;
				ch->iic_ir_mode_counter++;
			}
		}
	}
	
	return (READ_TYPE_MODE)ch->iic_ir_mode;
}
/*===================================================
            ȫ�ֺ���
��籦��==>��������ѯ��ʽ
pch:�ֵ�����
cmd:������
dataout:�ɹ��󷵻ص����� 
ms_timeout:��ʱms
once:��һ��(iic ir),ǰ����system.iic_ir_mode����
return :TRUE or FALSE   -1:����re�ߵȴ�
====================================================*/
int channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout,int ms_timeout,BOOL once)
{
	U8 iic_cmd = 0;//������iic�ļ��� ���� ָ���ǲ�һ����
	U16 buffer[8];
	U8 ch = channel_data_get_index(pch);    //������ 1 ��ʼ
	READ_TYPE_MODE mode = (READ_TYPE_MODE)pch->iic_ir_mode; //iic irģʽ   
	U8 dir = pch->iic_dir; 	//����
	U8 sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;
	U8 scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;
	int result = 0;
	int wanlen = 0;
	if(ch==0||pch==NULL)return FALSE;
	
	
	if(system.iic_ir_mode==SIIM_ONLY_IR)mode=RTM_IR;
	if(system.iic_ir_mode==SIIM_ONLY_IIC)mode=RTM_IIC;
	
	if(mode == RTM_IIC || (once&&system.iic_ir_mode!=SIIM_ONLY_IR) )//�Ƿ�����
	{
		//û��Ӧ��ı䷽���ٶ�һ��
		if(ld_bq27541_check_ack(sda,scl)==FALSE)
		{
			if(ld_bq27541_check_ack(sda,scl)==FALSE)
			{
				//�ı䷽��
				pch->iic_dir=!pch->iic_dir;
				dir = pch->iic_dir;//����
				sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;
				scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;		
				if(ld_bq27541_check_ack(sda,scl)==FALSE)
					if(ld_bq27541_check_ack(sda,scl)==FALSE){
						if(once==FALSE)
						{
							iic_ir_select_poll(pch,TRUE,FALSE);//ʧ��һ��
							pch->dingzhen_counter++;           //����ʶ�����++
						}
						else 
							 if(system.iic_ir_mode!=SIIM_ONLY_IIC)
								 goto READ_IR;//��һ��ir
						return FALSE;
					}		
			}
		}

		switch(cmd)
		{
			case RC_READ_ID:   result = ld_bq27541_read_id(sda,scl,dataout);
												 if(result==TRUE)memcpy(pch->id,dataout,CHANNEL_ID_MAX);
												
			break;
			case RC_READ_DATA: result = ld_bq27541_read_words(sda,scl,(U8*)TESTDATA1,4,buffer);
												 if(result==TRUE)//��ʽ�����
													{
														#define dat   ((U8*)(buffer))
														#define dat16 buffer
														U8 cs = 0xFF-cs8(dat,12);
														dataout[0]=0;
														dataout[1]=(U8)dat16[2];
														dataout[2]=(dat16[1]-2732)/10;
														dataout[3]=0;
														dataout[4]=dat[0];
														dataout[5]=dat[1];//ѭ������
														dataout[6]=dataout[7]=0;//����
														dataout[8]=dataout[9]=0;//��ѹ
														dataout[10]=dat[6];
														dataout[11]=dat[7];
														channel_save_data(pch,dataout);
													}	
													
			break;
			case RC_UNLOCK:if(iic_cmd==0)      iic_cmd=BAO_ALLOW;               //iicָ�� ��      05
		  case RC_UNLOCK_1HOUR:if(iic_cmd==0)iic_cmd=BAO_ALLOW_ONE_HOUR;//iicָ�� ��1Сʱ 07
			case RC_LOCK:if(iic_cmd==0)        iic_cmd=BAO_NOTALLOW;              //iicָ�� �����  06
			 result = ld_bq27541_de_encrypt_charge(sda,scl,iic_cmd);
			 if(result==TRUE)
				 result =ld_bq27541_output_flag(sda,scl,dataout);
			 break;
			default:return FALSE;		 									
		}
		
		if(result==TRUE){pch->dingzhen_counter=0;}                //����ʶ����0
		
		if(once==FALSE)
			iic_ir_select_poll(pch,(result==TRUE?FALSE:TRUE),FALSE);//��¼һ�Σ��Ƿ�Ҫ�л�
		else 
		{
			if(result==FALSE && (system.iic_ir_mode!=SIIM_ONLY_IIC))//�Ƿ������ٶ�һ�κ���
			 goto READ_IR;//��һ��ir
		}
		return result;
	}
	else if(mode==RTM_IR)
	{
		
		if(ld_gpio_get(pch->map->io_re))return -1;
		
		READ_IR:
		pch->iic_ir_mode=RTM_IR;
		switch(cmd)
		{
			case RC_READ_ID  :wanlen =  7;break;  //ʵ��406ms
			case RC_READ_DATA:wanlen = 13;break;  //ʵ��504ms
			default:wanlen = 2;
		}
		//��������
	  ld_ir_read_start(ch,0,cmd,wanlen);
		//�ȴ����� 
		while((result=ld_ir_read_isok(ch,(U8*)(buffer),wanlen))==1)
		{
			delayms(10);
			ms_timeout-=10;
			if(ms_timeout<0){
				result = 4;
				return FALSE;
			}
		}

		//������
	  if(result==2)
		{
			if(cmd==RC_READ_ID)  memcpy(pch->id,(U8*)(buffer),10);
			if(cmd==RC_READ_DATA)channel_save_data(pch,(U8*)(buffer));
			iic_ir_select_poll(pch,FALSE,FALSE);
			memcpy(dataout,(U8*)buffer,13);
			pch->ir_error_counter=0;
			delayms(10);//�ɹ�����ʱ10ms
			return TRUE;
		}
		//ʧ�ܺ���ʱ80ms
		else{
			delayms(120);
		}
		
		//����ʶ�����++
		pch->ir_error_counter++;
		{
			U8 pr[5]={ch};
			memcpy(pr+1,&pch->ir_error_counter,4);		
			enable_485_tx();;
		  ld_uart_send(COM_485,pr,5);
		}
		iic_ir_select_poll(pch,TRUE,FALSE);
		return FALSE;
	}
	
	return result;
}


