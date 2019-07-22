
/*===================================================
����:����Ŷ��㷨
�㷨����:

�ֵ�����: A.����Ϊ0�ֵ�    B.7Сʱ�ڳ��   C.�����ɿ��ܲ���

          1.���ȼ� A�� > B�� >C��
					2.���A�����ʱ���ȵ��ȵã��Ŷ�
					3.���B�����ʱ�������������Ŷӣ�
					4.
====================================================*/
#include "includes.h"

/*����(contiki����ռ,���Բ�����߳�ͬʱ����)*/
#define queue_lock()
#define queue_unlock()

#define l list[ch]

static BOOL inited = FALSE;

/*�Ŷӽṹ*/
#pragma pack(1)
typedef struct{
	U8 \
	used:1,    				//������:1   δ������:0
	charge:1,  				//������:1   ������:0
	hard:1,           //Ӧ����磬�������
	inited:1,         //��ʼ��
	reversed:4;				//δʹ��λ
	U8 ch;            //�ֵ��� 1-n
	U32 charge_time;	//���ʱ��(��)  				
}Queue_Type;
#pragma pack()

static Queue_Type list[CHANNEL_MAX]={0,0,0,0,0,0,0,0};    //�б�
/*===================================================
                ���غ���
====================================================*/
/*ð������:hard==1,value=200   hard==0,value=ʣ�����*/
static void bubble_sort(void)
{
	int va = 0,vb=0;
	int i=0,j=0;
  Channel*pcha,*pchb;
	for(;i<CHANNEL_MAX-1;i++)
	{
		for(j=i+1;j<CHANNEL_MAX;j++)
		{
			pcha = channel_data_get(list[i].ch);
			pchb = channel_data_get(list[j].ch);
			if(pcha==NULL || pchb==NULL)continue;
			va= (list[i].used)?( (list[i].hard)?(200):(pcha->Ufsoc)):(0);  //Ӧ�������ǰ
			vb= (list[j].used)?( (list[j].hard)?(200):(pchb->Ufsoc)):(0);  //֮�󣬵���������������
			if(va<vb)
			{
				//����
				Queue_Type t = list[i];
				list[i] = list[j];
				list[j] = t;
			}
		}
	}
}

/*ѡ��ǰ��ĳ��,���ص�ǰ������*/
static int charge_front(void)
{	
	int charge_counter=0; //��ǰ���ڳ��ĸ���
	int ch=0;           
	Channel*pch;
	for(;ch<CHANNEL_MAX;ch++) //ѡ��ǰ��ĳ��
	{
		pch = channel_data_get(ch+1);
		if(pch==NULL)continue;
		if(l.used)
		{
			if(charge_counter<CHANNEL_CHARGE_MAX)
			{	
				charge_counter++;
				set_out5v();
			}else reset_out5v();
		}else reset_out5v();
	}	
}

/*��ʱ*/
static void charge_timeout(void)
{
	static time_t t = 0;
	if(t==0)t=time(0)/1000;
	if( t != (time(0)/1000) )
	{
		int ch=0;           
		for(;ch<CHANNEL_MAX;ch++) //ѡ��ǰ��ĳ��
		{
			if(l.used)
			{
				if(l.charge_time!=0)
					l.charge_time--;
				else
					l.used=0;//ֹͣ���
			}
		}		
	}
}

/*�Ŷӵ����߳�*/
AUTOSTART_THREAD_WITH_TIMEOUT(queue)
{
  
  memset(list,0,sizeof(list));	
	PROCESS_BEGIN();
	while(1)
	{
		bubble_sort();          //����
		charge_timeout();			  //��ʱ
		charge_front();         //ǰ���ȳ�
    os_delay(queue,20);     
	}

	PROCESS_END();
}

static void request_init(void)
{
	memset(list,0,sizeof(list));
	{
		int ch = 0;
		for(;ch<CHANNEL_MAX;ch++)
		{
			l.inited=1;
			l.ch=ch+1;
		}
	}
}
/*===================================================
                ȫ�ֺ���
====================================================*/

/*������*/
BOOL request_charge_on(U8 ch,U32 seconds,BOOL hard)
{
	if(!inited)request_init();
	if(ch==0 || ch>= CHANNEL_MAX)return FALSE;ch--;
  
}

/*��ֹ���*/
BOOL request_charge_off(U8 ch)
{
	if(!inited){request_init();return TRUE;}

}

/*������*/
BOOL request_charge_hangup_all(U32 seconds)
{
	if(!inited){request_init();return TRUE;}
}
