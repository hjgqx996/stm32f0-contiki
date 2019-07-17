#include "includes.h"

/*===================================================
                ��籦��-״̬��
			ͨ��������Ϣ�ķ�ʽ��ͳһiic,ir��д�ӿ�
			50ms�ĳ���ʱ��״̬��α����
====================================================*/
#define READ_TYPE_DATA_MAX  16
/*  ���� */
typedef enum{
	READ_TYPE_READ_ID,        //:�����ID
	READ_READ_DATA,           //:������
	READ_READ_UNLOCK,         //:����05
	READ_READ_LOCK,           //:����06
	READ_UNLOCK_1HOUR,        //:����1Сʱ07
	READ_OUTPUT,              //:�������־
}READ_TYPE_CMD;//��籦����


/*��д��ʽ*/
typedef enum{
	READ_TYPE_MODE_IIC=0,
  READ_TYPE_MODE_IR,
}READ_TYPE_MODE;

/*��д����*/
typedef struct{  
	READ_TYPE_MODE mode;  // 0:iic   1:����
	//===========iic����================//
	U8 sda;
	U8 scl;
	//===========����===================//
  U8 ch;//1-n
	//===========����===================//
	READ_TYPE_CMD cmd;
}Read_Type_Ctrl;

/*��Ϣ�ṹ��*/
typedef struct{
	BOOL used;            //TRUE :��Ч
	BOOL start;           //FALSE:���   TRUE:��ʼ
	Read_Type_Ctrl ctrl;  //����
	U8 data[READ_TYPE_DATA_MAX];            //���ݻ���
	FSM fsm;              //״̬��
}Read_Type;

static Read_Type rts[CHANNEL_MAX * 2];
static void fsm_read(Read_Type*rt,FSM*fsm)
{
	fsm_time_set(time(0));
	
	Start(��ʼ){
		
		if(rt->used==TRUE && rt->start==TRUE)
		{
			
		}
		
		
		//iic
		if(rt->ctrl.mode==READ_TYPE_MODE_IIC)
		{
		
		}
		else if(rt->ctrl.mode == READ_TYPE_MODE_IR)
		{
			
		}	
	}
	

}

/*===================================================
                ��籦������
====================================================*/
static struct etimer et_bao;
PROCESS(thread_bao, "ͨ������");
AUTOSTART_PROCESSES(thread_bao);
PROCESS_THREAD(thread_bao, ev, data)  
{
	PROCESS_BEGIN();
  channel_data_init();//��ʼ���ֵ�����
	while(1)
	{
    U8 i=0;
		for(;i<CHANNEL_MAX*2;i++)
		{
			if(rts[i].used){fsm_read(rts+i,&(rts+i)->fsm);}//�������в���״̬��
		}
		os_delay(et_bao,10);
	}
	PROCESS_END();
}

/*===================================================
                ��籦������ӿ�
====================================================*/
/*��ȡһ����籦������*/
BOOL ld_bao_read_start(Read_Type_Ctrl ctrl)
{

}
/*��ѯ�Ƿ���ɹ�, �ɹ��ͷ�������*/
BOOL ld_bao_read_isok(Read_Type_Ctrl ctrl,U8*dataout)
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
	PROCESS_BEGIN();
  channel_data_init();//��ʼ���ֵ�����
	while(1)
	{

	}

	PROCESS_END();
}


