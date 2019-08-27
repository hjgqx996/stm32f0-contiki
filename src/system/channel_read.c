
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
====================================================*/
#include "includes.h"


#define force_using_iic()  (system.iic_ir_mode==SIIM_ONLY_IIC)
#define force_using_ir()   (system.iic_ir_mode==SIIM_ONLY_IR)



/*===================================================
                ���ر���
====================================================*/
const unsigned char TESTDATA1[] =
{
0x2a,      0x06, 0x04,           0x14,
//ѭ������ /�¶� /ʣ������           /����
};

/*===================================================
            ȫ�ֺ���
��籦��==>������ѯ��ʽ
pch:�ֵ�����
cmd:������
dataout:�ɹ��󷵻ص����� 
ms_timeout:��ʱms
once:=true:
						�Ȱ�ԭ����ͨѶ��ʽͨѶ,���ʧ�ܣ���ʹ�������ͨѶ��ʽ
return :TRUE or FALSE   -1:����re�ߵȴ�
====================================================*/
int channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout,int ms_timeout,BOOL once)
{
	U8 iic_cmd = 0;//������iic�ļ��� ���� ָ���ǲ�һ����
	U16 buffer[8];//��������
	U8 ch = channel_data_get_index(pch);    //������ 1 ��ʼ
	U8 first_mode; //��һ��ͨѶ��ʽ
	#define mode  pch->iic_ir_mode
	int result = 0;
	int wanlen = 0;
	if(ch==0||pch==NULL)return FALSE;
	if(system.iic_ir_mode==SIIM_ONLY_IR)mode=RTM_IR;
	if(system.iic_ir_mode==SIIM_ONLY_IIC)mode=RTM_IIC;
	
	if(mode == RTM_IIC){first_mode=RTM_IIC;goto IIC_READ_MODE_OPERATION;}//ʹ��iic
	if(mode == RTM_IR) {first_mode=RTM_IR;goto IR_READ_MODE_OPERATION;  }//ʹ��ir
	
		/*------------------------iic ��ʽ ��ȡ -----------------------------------*/
		IIC_READ_MODE_OPERATION:
	  mode=RTM_IIC;
		{
				U8 dir = pch->iic_dir; 	//����
				U8 sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;
				U8 scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;
				//���Ӧ��,��������
				if((result=ld_bq27541_check_ack(sda,scl))==FALSE)
				{
					delayms(10);//��ʱ10ms ==>�����
					if((result=ld_bq27541_check_ack(sda,scl))==FALSE)
					{	
						/*�ı䷽�򣬼��Ӧ�𣬳�������*/
						pch->iic_dir=!pch->iic_dir;//�ı䷽��
						dir = pch->iic_dir;        //����
						sda = (dir==1)?pch->map->io_scl:pch->map->io_sda;//sda
						scl = (dir==1)?pch->map->io_sda:pch->map->io_scl;//scl		
						delayms(10);//��ʱ10ms ==>�����
						if((result=ld_bq27541_check_ack(sda,scl))==FALSE)
						{
							delayms(10);//��ʱ10ms ==>�����
							if((result=ld_bq27541_check_ack(sda,scl))==FALSE){ 
								goto IIC_READ_ERROR;//��ʧ�� 
							}
						}					
					}
				}
				//IIC��
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
					case RC_UNLOCK:      if(iic_cmd==0)iic_cmd=BAO_ALLOW;         //iicָ�� ��      05
					case RC_UNLOCK_1HOUR:if(iic_cmd==0)iic_cmd=BAO_ALLOW_ONE_HOUR;//iicָ�� ��1Сʱ 07
					case RC_LOCK:        if(iic_cmd==0)iic_cmd=BAO_NOTALLOW;      //iicָ�� �����  06
					 result = ld_bq27541_de_encrypt_charge(sda,scl,iic_cmd);
					 if(result==TRUE)
						 result =ld_bq27541_output_flag(sda,scl,dataout);           //�������־
					 break;
					default:return FALSE;		 									
				}
				/*iic ��ʧ��*/
				IIC_READ_ERROR:
				if(result==FALSE){
					
		//			U8 pb[8];pb[0]=RTM_IIC; pb[1]=pch->addr;
		//			enable_485_tx();//ʹ�ܷ���
		//			ld_uart_send(COM_485,pb,2);//��ӡ���
				
					pch->iic_error_counter++;//����ʶ�����++
					if(!force_using_ir())//��ǿ��ʹ��iic
					{
						if((once==TRUE)&&(first_mode==RTM_IIC))//once==true,��һ��ir
						{
							delayms(50);
							goto IR_READ_MODE_OPERATION;
						}
					}
					if(pch->iic_error_counter>=BAO_DINGZHEN_ERROR_TIMES) //���δ�������һ�κ���
					{
						if(!force_using_iic())   //��ǿ��ʹ��iic
							mode=RTM_IR;
					}
					return FALSE;
				}else
					pch->iic_error_counter=0;
				
				return result;
	  }
	
	/*------------------------���� ��ʽ ��ȡ -----------------------------------*/
	IR_READ_MODE_OPERATION:
	{
		if(ld_gpio_get(pch->map->io_re))return -1;/*���ⱻ���ߣ���ʾ����æ��ֱ�ӷ���-1*/
		mode=RTM_IR;
		switch(cmd)
		{
			case RC_READ_ID  :wanlen =  7;break;  //ʵ��406ms
			case RC_READ_DATA:wanlen = 13;break;  //ʵ��504ms
			default:wanlen = 2;
		}
		
	  ld_ir_read_start(ch,FALSE,cmd,wanlen);  //��������
		
		while((result=ld_ir_read_isok(ch,(U8*)(buffer),wanlen))==1)//�ȴ�����,��ʱʱ��ms_timeout ms
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
			if(cmd==RC_READ_ID)  memcpy(pch->id,(U8*)(buffer),10);     // copy id
			if(cmd==RC_READ_DATA)channel_save_data(pch,(U8*)(buffer)); // copy data
			memcpy(dataout,(U8*)buffer,2);                             //��������
			pch->ir_error_counter=0;                                   //��������־��
			delayms(10);                                               //�ɹ�����ʱ10ms
			return TRUE;
		}
		else
		{
			delayms(50);                                              //ʧ�ܺ���ʱ120ms
		}
		
		//����ʶ�����++
		pch->ir_error_counter++;
		
		if(!force_using_ir())//��ǿ��ʹ��ir
		{
			if((once==TRUE)&&(first_mode==RTM_IR))//once==true,��һ��iic
			{
				delayms(50);
			  goto IIC_READ_MODE_OPERATION;
			}
		}
		
		/*ʧ������תiic*/
		if(pch->ir_error_counter>=BAO_IR_ERROR_TIMES){
			if(!force_using_ir())//��ǿ��ʹ��IR       
				mode=RTM_IIC;
		}
		
//		{
//			U8 pb[8];pb[0]=RTM_IR; pb[1]=pch->addr;
//			enable_485_tx();//ʹ�ܷ���
//			ld_uart_send(COM_485,pb,2);//��ӡ���
//		}
		return FALSE;
	}
	
	return result;
}


