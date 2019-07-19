#include "types.h"
#include "dev.h"
#include "string.h"
#include "lib.h"
/*===================================================
                �����ļ�
====================================================*/
#define sda_out()
/**************************************
	private ģ��i2c����
**************************************/
#define SCL_H()			ld_gpio_set(scl,HIGH)
#define SCL_L()			ld_gpio_set(scl,LOW)

#define SDA_H()			ld_gpio_set(sda,HIGH)
#define SDA_L()			ld_gpio_set(sda,LOW)

#define SDA_READ()		ld_gpio_get(sda)
#define SCL_READ()		ld_gpio_get(scl)

//extern void cpu_us_delay(int us);
//#define delayus(n)   cpu_us_delay(n)

#define i2c_start()     		I2C_Start(sda,scl)
#define i2c_restart()   		I2C_Restart(sda,scl)
#define i2c_stop()      		I2C_Stop(sda,scl)
#define i2c_ack()       		I2C_Ack(sda,scl)
#define i2c_noack()         I2C_NoAck(sda,scl)
#define i2c_wait_ack()      I2C_WaitAck(sda,scl)
#define i2c_send_byte(data) I2C_Send_Byte(sda,scl,data)
#define i2c_read_byte()     I2C_Read_Byte(sda,scl)
#define i2c_check_ack()     if(i2c_wait_ack()==FALSE) {i2c_stop();return FALSE;}



//�ж�ʱ�����Ƿ�Ϊ��
//������ȴ�һ��ʱ�䣬���950us����
//2017-7-21
void wait_scl_high(U8 scl)
{
	U16 c=100; //100us��ʱ
	while(c>0)
  {
		 delayus(1);
		 if(SCL_READ()==1){
			  c=c;
		    return ;
		 }
     c--;
  }
}

static void I2C_Start(U8 sda,U8 scl)
{
  sda_out();
	SDA_H();
	SCL_H();
 	delayus(5);	
//	wait_scl_high(scl);	
// 	delayus(5);
	SDA_L();
	delayus(5);
	SCL_L();
	delayus(5);
}

static void I2C_Restart(U8 sda,U8 scl)
{
  sda_out();
	SCL_L();
	delayus(5);
	SDA_H();
	delayus(5);
	SCL_H();
	wait_scl_high(scl);	
 	delayus(5);
	SDA_L();
	delayus(5);
	SCL_L();
	delayus(5);
}

static void I2C_Stop(U8 sda,U8 scl)
{
	sda_out();
	SDA_L();
	delayus(5);
	SCL_H();
	delayus(5);
	SDA_H();
	delayus(5);
}

static void I2C_Ack(U8 sda,U8 scl)
{	
	SDA_L();
	delayus(2);
	SCL_H();
	delayus(6);
	SCL_L();
	SDA_H();
	delayus(2);
}

static void I2C_NoAck(U8 sda,U8 scl)
{	
	SDA_H();
	delayus(5);
	SCL_H();
	delayus(5); //BQ2754��ʱ5us
	SCL_L();
	SDA_L();
	delayus(5);
}

static BOOL I2C_WaitAck(U8 sda,U8 scl)	 //����Ϊ:=TRUE��ACK,=FALSE��ACK
{
	U8 data=0;
	SDA_H();			
	SCL_H();
	delayus(5);
	wait_scl_high(scl);
	data=SDA_READ();
	SCL_L();
	delayus(5);
  return (data==1)?FALSE:TRUE;//sda==0,��Ӧ��
}

static void I2C_Send_Byte(U8 sda,U8 scl,U8 ucData) //���ݴӸ�λ����λ//
{
	U8 i=0,temps,dat;
	temps=ucData;
	dat=0x80;
	for(i=0;i<8;i++)
	{
		if(dat&temps)
		{
			SDA_H();
			delayus(2);
			SCL_H();
			delayus(2);
			SCL_L();
			delayus(2);
		}else{
			SDA_L();
			delayus(2);
			SCL_H();
			delayus(2);
			SCL_L();
			delayus(2);
		}
		dat=dat>>1;
	}
	SDA_H();
	delayus(1);
}

static  U8 I2C_Read_Byte(U8 sda,U8 scl)  //���ݴӸ�λ����λ//
{ 
	U8 i=8;
	U8 ucData=0;
	SDA_H();				
	while(i--)
	{
		ucData<<=1;      
		SCL_L();
		delayus(5);
		SCL_H();
		wait_scl_high(scl);
		delayus(5);	
		if(SDA_READ())
		{
			ucData|=0x01;
		}
	}
	SCL_L();
	delayus(4);
	return ucData;
}

/*===================================================
                ˽��BQ27541����
====================================================*/
#define BQ27541_ADD_WR 		0xAA        //����iic����д��ַ
#define BQ27541_ADD_RD 		0xAB        //����iic��������ַ

/*��һ����
* cmd: �����ֽڣ�����ָ�Ĵ�����ַ
* dataout:���ص�����
*/
static BOOL bq27541_read_word(U8 sda,U8 scl,U8 cmd,U16 *dataout)
{
	U8 L,H;
	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack() //2017-7-21
	i2c_send_byte(cmd);
	i2c_check_ack() //2017-7-21
	i2c_restart();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack() //2017-7-21
	L=i2c_read_byte();//���ֽ�
	i2c_ack();
	H=i2c_read_byte();//���ֽ�
	i2c_noack();
	i2c_stop();
	*dataout=(((U16)H)<<8)|L;
	return TRUE;
}

/*��һ���ֽ�
* cmd: �����ֽڣ�����ָ�Ĵ�����ַ
* dataout:���ص�����
*/
static BOOL bq27541_read_byte(U8 sda,U8 scl,U8 cmd,U8 *dataout)
{
	U8 temp;
	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack() //2017-7-21
	i2c_send_byte(cmd);
	i2c_check_ack() //2017-7-21
	i2c_restart();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack() //2017-7-21
	temp=i2c_read_byte();
	i2c_noack();
	i2c_stop();
	*dataout=temp;
	return TRUE;
}

//��ȡ����ֽ�
static BOOL bq27541_read_power(U8 sda,U8 scl,U8*dataout)
{
	int i = 0;
	for(;i<10;i++)
	{
		if(bq27541_read_byte(sda,scl,(0x40+(9-i)),dataout)==FALSE)
			return FALSE;
		else
			dataout++;
	}
	return TRUE;
}

/*The bq27541 detects whether the SMBus enters the Off State by monitoring the SMBC and SMBD lines. When
both signals are continually low for at least 2.0 s, the bq27541 detects the Off State. When the SMBC and SMBD
lines go high, the bq27541 detects the On State and can begin communication within 1 ms. */
static void bq27541_smb_on(U8 sda,U8 scl)
{
	sda_out();
	SDA_H();
	SCL_H();
	delayus(5000); //����ȡ�������ⲿ��ʱ
}

/*===================================================
                ȫ�ֺ���
====================================================*/

/*
* ��ȡbq25741 n����
* sda :sda �˿ں�
* scl :scl �˿ں�
* cmd :��������
* cmdlen:�������鳤��
* dataout: �������
*return  : TRUE or FALSE
*/
BOOL ld_bq27541_read_words(U8 sda,U8 scl,U8*cmd,U8 cmdlen,U16 *dataout)
{
	if(cmd==NULL||dataout==NULL||cmdlen>100)return FALSE;
	{
		int i=0;
		for(;i<cmdlen;i++)
		{
			if(bq27541_read_word(sda,scl,cmd[i],dataout)==FALSE)
				return FALSE;
			dataout++;
		}
	}
	return TRUE;
}

/*
* ��ȡbq25741 id
* sda :sda �˿ں�
* scl :scl �˿ں�
* dataout: �������
*return  : TRUE or FALSE
*  ��read_id�ֳ�������:
*  start----50ms----end
*  ԭ����50msӲ��ʱ���ܻ�������غ��
*/
BOOL ld_bq27541_read_id_start(U8 sda,U8 scl)
{
 	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x3F); 
  i2c_check_ack()
	i2c_send_byte(0x01); 
  i2c_check_ack()
	i2c_stop();
	return TRUE;
}
BOOL ld_bq27541_read_id_end(U8 sda,U8 scl,U8*dataout)
{
	U8 temp[10],cs,byte;
	//��ȡ����ֽ�
	if(bq27541_read_power(sda,scl,temp)==FALSE)
	{
		i2c_stop();//��ʵ�����Ѿ���bq27541_read_power�д���
		return FALSE;
	}
	//����
	cs = 0xFF-cs8(temp,10);
	if(cs!=0xFF){
		bq27541_read_byte(sda,scl,0x60,&byte);
		if(byte!=cs){
			//ʧ��6�Σ���籦��� ��0
			return FALSE;
		}
		else
			memcpy(dataout,temp,10);
	}else{
		memcpy(dataout,temp,10);
	}
	return TRUE;
}


/*
* ���� ���� ��籦
* sda :sda �˿ں�
* scl :scl �˿ں�
* cmd :05:������ 06:������ 07:����1Сʱ
*return  : TRUE or FALSE
*  ��read_id�ֳ�������:
*  start----50ms----end
*  ԭ����50msӲ��ʱ���ܻ�������غ��
*/
BOOL ld_bq27541_de_encrypt_charge_start(U8 sda,U8 scl,U8 cmd)
{
	U8 data[2];
 	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x62); 
  i2c_check_ack()
	i2c_stop();
	
	delayus(5);
	
	i2c_start();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack()
	data[0]= i2c_read_byte();
	i2c_ack();
	data[1]= i2c_read_byte(); 
	i2c_stop();
	
	delayus(5);

 	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x71); 
  i2c_check_ack()
	i2c_send_byte(cmd); 
  i2c_check_ack()
	i2c_stop();
	
	return TRUE;	
}
BOOL ld_bq27541_de_encrypt_charge_end(U8 sda,U8 scl)
{
	U8 data = 0;
	i2c_start();
  i2c_send_byte(BQ27541_ADD_WR); 
	i2c_check_ack()
	i2c_send_byte(0x71); 
  i2c_check_ack()
	i2c_stop();
	
	delayus(5);
	
	i2c_start();
	i2c_send_byte(BQ27541_ADD_RD); 
	i2c_check_ack()
	data= i2c_read_byte();
	i2c_noack();
	i2c_stop();
	return TRUE;
}


/*bq27541���Ӧ��
*���ܣ���ʼ������0x62���Ҫ��
*/
BOOL ld_bq27541_check_ack(U8 sda,U8 scl)
{
	
	U16 data = 0;
	if(bq27541_read_word(sda,scl,0x62,&data)==FALSE)return FALSE;
	if(data!=0x6207)return FALSE ;
	return TRUE;
}

/*bq27541��籦�����־*/
BOOL ld_bq27541_output_flag(U8 sda,U8 scl,U8*data)
{
	U16 tmp;
	if(bq27541_read_word(sda,scl,0x62,&tmp)==FALSE)return FALSE;
	if(bq27541_read_byte(sda,scl,0x71,(U8*)&tmp)==FALSE)return FALSE;
	*data=(U8)tmp;
	return TRUE;
}

/*===================================================
                ��׼���ӿڣ������һ���Ľӿ�
====================================================*/
#include "channel.h"
#define IIC_DATA_MAX      16
#define IIC_CHANNEL_MAX   5

/*״̬*/
typedef enum{	
	IIC_State_Error=-1,
	IIC_State_NULL=0,     
	IIC_State_OK   =2,       //��ȡ������ȷ
}IIC_STATE;

typedef struct{
	U8 sda;        			//���ⷢ��io
	U8 scl;        			//�������io
	BOOL opposite;      //����
	READ_TYPE_CMD cmd;  //��������	
	U8 wanlen;       	  //Ҫ���յ����ݳ���
	U8 len;          		//ʵ�ʽ��յ������ݳ���
	BOOL start;      		//TRUE: ��ʼ  FALSE:����
	IIC_STATE state;    //������		
	S32 counter;     		//����
	U16 data[IIC_DATA_MAX/2]; //��������
	U8 tmp;          			//����һ�ֽ�
	
	BOOL inited;          //��ʼ����־��δ��ʼ����״̬����������
	////////////////////////////////
	FSM fsm;         //״̬��˽�б���
}IIC_Type;
static IIC_Type iics[IIC_CHANNEL_MAX];
#define iic_lock()
#define iic_unlock()
#define sda_port()   ((piic->opposite)?(piic->scl):(piic->sda))
#define scl_port()   ((piic->opposite)?(piic->sda):(piic->scl))

const unsigned char IIC_DATA_CMDS[] ={//ѭ������ /�¶� /ʣ������  /����
																		0x2a,    0x06, 0x04,      0x14,
};

/*
*  iic ״̬��
*  ����: ��iic�е�50msӲ��ʱ�ͷų�������cpu��������������
*  �ӿ�ʵ��:   ld_iic_read_start ����״̬�����ж�
*              ld_iic_read_isok  �鿴״̬���Ƿ���ɶ�����
*  ״̬��ʵ��:
           ��ʼ--->���Ӧ��---->
              (1)��id           ld_bq27541_read_id_start---->��ʱ50ms(״̬���ͷ�cpu)---->ld_bq27541_read_id_end
              (2)������         ld_bq27541_read_words
              (3)�ӽ���         ld_bq27541_de_encrypt_charge_start---->��ʱ50ms(״̬���ͷ�cpu)---->ld_bq27541_de_encrypt_charge_end
              (4)�����         ld_bq27541_output_flag
*  ��Ҫ����״̬�������벻�����룬����ʹ���߳�   AUTOSTART_THREAD_WITH_TIMEOUT(iic)
   ��ʵ��״̬���Ĳ�������
*/
static void iic_fsm(IIC_Type*piic,FSM*fsm)
{
	U8 tmp=0;
	fsm_time_set(time(0));
	
	Start(��ʼ)
	{
		if((piic==NULL) || (piic->inited==FALSE) || (piic->inited==FALSE))return;
		if(piic->start)
		{
			
			if(ld_bq27541_check_ack(sda_port(),scl_port()))//���Ӧ��
			{
				switch(piic->cmd)
				{
					//��ic
					case RC_READ_ID:
						if(ld_bq27541_read_id_start(sda_port(),scl_port()))
						{goto ��ʼ��id��ʱ50ms;}
						else 
							goto IIC_FSM_Error;
						
					//������
					case RC_READ_DATA:
						if(ld_bq27541_read_words(sda_port(),scl_port(),(U8*)IIC_DATA_CMDS,4,piic->data))
						{ piic->len=8; goto IIC_FSM_Sucess;}
						else
							goto IIC_FSM_Error;
						
					//�ӽ���	
					case RC_LOCK:if(tmp==0)tmp=0x06;
					case RC_UNLOCK:if(tmp==0)tmp=0x05;
					case RC_UNLOCK_1HOUR:if(tmp==0)tmp=0x07;
						if(ld_bq27541_de_encrypt_charge_start(sda_port(),scl_port(),tmp))
						{goto ��ʼ�ӽ�����ʱ50ms;}
						else 
							goto IIC_FSM_Error;
						
					//�����־
					case RC_OUTPUT:
						if(ld_bq27541_output_flag(sda_port(),scl_port(),(U8*)piic->data))
						{piic->len=1;goto IIC_FSM_Sucess;}
						else 
							goto IIC_FSM_Error;
					default:
						goto IIC_FSM_Error;
				}			
				
			}
			else
			{//���Ӧ��ʧ��
			  goto IIC_FSM_Error;
			}				
		}
	}
	
	State(��ʼ��id��ʱ50ms){waitmsx(50);goto ������id;}
	State(������id)
	{
	  if(ld_bq27541_read_id_end(sda_port(),scl_port(),(U8*)piic->data))
		{
			piic->len=13;
			goto IIC_FSM_Sucess;
		}
		else
			goto IIC_FSM_Error;
	}
	
	State(��ʼ�ӽ�����ʱ50ms){waitmsx(50);goto �����ӽ���;}
	State(�����ӽ���)
	{
		if(ld_bq27541_de_encrypt_charge_end(sda_port(),scl_port()))
		{
			if(ld_bq27541_output_flag(sda_port(),scl_port(),(U8*)piic->data))//��ȡ��־
				goto IIC_FSM_Sucess;
			else 
				goto IIC_FSM_Error;
		}	
		else 
			goto IIC_FSM_Error;
	}
	
	Default()
	return ;
	
	IIC_FSM_Error:
	 piic->start=FALSE;
	 piic->state=IIC_State_Error;
	 memset(&piic->fsm,0,sizeof(FSM));
	 return;
	IIC_FSM_Sucess:
	 piic->start=FALSE;
	 piic->state=IIC_State_OK;
	 memset(&piic->fsm,0,sizeof(FSM));
	 return;
}

#include "contiki.h"
AUTOSTART_THREAD_WITH_TIMEOUT(iic)
{
	PROCESS_BEGIN();
	while(1)
	{
		static U8 i = 0;
		for(i=0;i<IIC_CHANNEL_MAX;i++)
		{
			if((iics[i].inited==TRUE) && (iics[i].start==TRUE))
			{
				iic_fsm(iics+i,&(iics+i)->fsm);
			}
		}
		os_delay(iic,10);
	}
	PROCESS_END();
}

/*===================================================
                ȫ�ֺ���
====================================================*/
/*��ʼ������
* ch    :�ֵ��� 1-n
* sda :���ݶ˿�
* scl :ʱ�Ӷ˿�
*/
void ld_iic_init(U8 ch,U8 sda,U8 scl)
{
	ch-=1;
	iic_lock();
	if(ch>=IIC_CHANNEL_MAX){
		iic_unlock();return;
	}
  memset(&iics[ch],0,sizeof(IIC_Type));
	iics[ch].sda=sda;
	iics[ch].scl=scl;
	iics[ch].start=FALSE;
	iics[ch].state=IIC_State_NULL;
	iics[ch].inited = TRUE;
	iic_unlock();
}
//������
BOOL ld_iic_read_start(U8 ch,BOOL opposite,U8 cmd,U8 wanlen)//(ch:1-n,opposite:TRUE����, cmd ����, ����)
{
 	ch-=1;
	iic_lock();
	if( (ch>=IIC_CHANNEL_MAX) || (iics[ch].inited==FALSE) ){
		iic_unlock();return FALSE;
	}
	iics[ch].start=TRUE;
	iics[ch].state=IIC_State_NULL;
	iics[ch].cmd = (READ_TYPE_CMD)cmd;
	iics[ch].wanlen=wanlen;
	iics[ch].opposite=opposite;
	iic_unlock();

}
//�Ƿ�æ
BOOL ld_iic_busy(U8 ch)
{
	BOOL r = 0;
	ch-=1;
	iic_lock();
	if(ch>=IIC_CHANNEL_MAX){iic_unlock();return FALSE;}
	r = iics[ch].start;
	iic_unlock();
	return r;
}
//�Ƿ�����
BOOL ld_iic_cmd(U8 ch,U8 cmd)
{
	ch-=1;
	iic_lock();
	if(ch>=IIC_CHANNEL_MAX){iic_unlock();return FALSE;}
	iic_unlock();
	return (cmd==iics[ch].cmd);
}

/*�鿴�Ƿ�����
* return : <0��error
*        :  0: �޲���
*        :  1: ���ڶ�
*        :  2: ����ȷ
*/
int ld_iic_read_isok(U8 ch,U8*dataout,U8 size)
{
	int err = -1;
	ch-=1;
  iic_lock();
	if(ch>=IIC_CHANNEL_MAX)goto END;
	if(iics[ch].start==FALSE)
	{
		err=(int)iics[ch].state;
	}else{
		err = 1;
	}
	//��ȷʱ����������:��ʽ�����
	if(err==IIC_State_OK && dataout!= NULL)
	{
		//��ʽ�������������
		switch(iics[ch].cmd)
		{
			case RC_READ_ID:
				memcpy(dataout,iics[ch].data,10);
	    break;
			
			case RC_READ_DATA://[0] �汾�� [1] ���� [2] �¶� [3] ������ [4-5] ѭ������ [6-7] ���� [8-9] ��о��ѹ [10-11] ���� (��λ��ǰ)
			{
				#define dat   ((U8*)(iics[ch].data))
				#define dat16 iics[ch].data
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
			}break;
			
			case RC_LOCK:         
			case RC_UNLOCK:
			case RC_UNLOCK_1HOUR:
			case RC_OUTPUT:      //[0]�����־
				dataout[0] = dat[0];
			break;
      default:goto END;
		}
	}
	END:
	iic_unlock();
	return err;
}

///*===================================================
//                IIC����
//====================================================*/
//AUTOSTART_THREAD_WITH_TIMEOUT(iic_test)
//{
//	U8 dataout[13];
//	PROCESS_BEGIN();
//	while(1)
//	{
//		
//		ld_iic_read_start(2,FALSE,RC_READ_ID,10);
//		os_delay(iic_test,200);
//		ld_iic_read_isok(2,dataout,10);
//		os_delay(iic_test,10);
//		
////		ld_iic_read_start(2,FALSE,RC_READ_DATA,13);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,13);
////    os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_LOCK,0);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,0);
////		os_delay(iic_test,0);
////		
////		ld_iic_read_start(2,FALSE,RC_OUTPUT,1);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,1);
////		os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_UNLOCK,0);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,0);
////		os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_OUTPUT,1);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,1);
////		os_delay(iic_test,10);

////		ld_iic_read_start(2,FALSE,RC_UNLOCK_1HOUR,0);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,0);
////		os_delay(iic_test,10);
////		
////		ld_iic_read_start(2,FALSE,RC_OUTPUT,1);
////		os_delay(iic_test,200);
////		ld_iic_read_isok(2,dataout,1);
////		os_delay(iic_test,10);

//	}
//	PROCESS_END();
//}
