
#include "includes.h"

/*�Ƿ�ǿ�Ƴ��*/
#define is_force_charge_on(ch)    ((system.mode==1) && (system.chs[ch-1]==0x01))   //ǿ�Ƴ��
#define is_force_charge_off(ch)   ((system.mode==1) && (system.chs[ch-1]==0x00))   //ǿ�ƶϵ�

/*����(contiki����ռ,���Բ�����߳�ͬʱ����)*/
#define queue_lock()
#define queue_unlock()

#define l list[ch]
static BOOL inited = FALSE;   //�Ƿ��ʼ��
static BOOL hangall = FALSE;  //�Ƿ����
static U32  hangtime = 0;     //����ʱ�� ms

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
/*
*  ֻ��һ��hard����ǰ��:��֤��һ�����ڳ��
*  [hard] [......] [hard][hard]...
*/
/*ð������*/
static void bubble_sort(void)
{
	int va = 0,vb=0;
	int i=0,j=0;
  Channel*pcha,*pchb;
	BOOL first = FALSE;
	for(;i<CHANNEL_MAX-1;i++)
	{
		for(j=i+1;j<CHANNEL_MAX;j++)
		{
			pcha = channel_data_get(list[i].ch);
			pchb = channel_data_get(list[j].ch);
			if(pcha==NULL || pchb==NULL)continue;
			va= (list[i].used)?( (list[i].hard)?((first)?(1):(220)):(pcha->Ufsoc*2)):(0);  //Ӧ�������ǰ,����ֻ��һ��,֮�����ĩβ		(Ȩ��)
			vb= (list[j].used)?( (list[j].hard)?((first)?(1):(220)):(pchb->Ufsoc*2)):(0);  //֮�󣬵���������������                 (Ȩ��)
			if(va<vb)
			{
				Queue_Type t = list[i];//����
				list[i] = list[j];
				list[j] = t;
			}
		}
		if(list[0].used && list[0].hard && first==FALSE)first=TRUE;
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
		pch = channel_data_get(l.ch);
		if(pch==NULL)continue;
		if(l.used && (hangall==FALSE))//��Ч and ������
		{
			if(charge_counter<CHANNEL_CHARGE_MAX)
			{	
				charge_counter++; //��ǰ����++
				l.charge=1;       //���ڳ��
				set_out5v();      //���5V
				continue;
			}
		}			
		//�����:δ�ŵ�,�����
		reset_out5v();
		l.charge=0;
	}	
	return charge_counter;
}

/*��ʱ*/
static void charge_timeout(void)
{
	static time_t t = 0;
	if(t==0)t=time(0)/1000;
	if( t != (time(0)/1000) )
	{
		t = time(0)/1000;
		//��������ʱ
		if(hangall==FALSE)
		{
			int ch=0;           
			for(;ch<CHANNEL_MAX;ch++) //ѡ��ǰ��ĳ��
			{
				if(l.used&&l.charge)    //����ʱ,����粻��ʱ
				{
					if(l.charge_time!=0)
						l.charge_time--;
					else
					{
						l.used=0;//ֹͣ���
					}
				}
			}		
		}
		//���𵹼�ʱ
		else{
			if(hangtime>0)hangtime--;
		}	
	}
	
	//����ʱ���ָ�����ʱ
	if(hangtime==0)hangall=FALSE;
}

/*���г�ʼ��*/
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
	inited=TRUE;
}

/*����Ԫ��*/
static Queue_Type*request_channel_find(U8 channel)
{
	if(!inited)request_init();
	{
		int ch = 0;
		for(;ch<CHANNEL_MAX;ch++)
		{
			if(l.ch==channel)return &l;
		}
	}
	return NULL;
}

/*ֱ�����*/
static BOOL direct_charge(U8 ch,BOOL charged)
{
	Channel*pch = channel_data_get(ch);
	if(pch==NULL)return FALSE;
	if(charged==FALSE)reset_out5v();
	else set_out5v();
	return TRUE;
}
/*===================================================
                ȫ�ֺ���
====================================================*/

/*������*/
BOOL request_charge_on(U8 ch,U32 seconds,BOOL hard)
{
	Queue_Type *qt = request_channel_find(ch);
	if(!inited)request_init();//δ��ʼ����Ӧ�ó�ʼ��
	if(qt==NULL)return FALSE;
  qt->hard=hard;            //hard=1:��ʾ5��or10min�������
	qt->charge_time = seconds;//+(((time(0)%1000)>500)?1:0);//������ʱ��
	qt->used=1;               //��Ч��־ 
	return TRUE;
}

/*��ֹ���*/
BOOL request_charge_off(U8 ch)
{
	if(!inited)request_init();
	Queue_Type *qt = request_channel_find(ch);
	qt->charge=qt->hard=qt->used=0;//��λ��Ч��־
	qt->charge_time=0;
	{
		Channel*pch = channel_data_get(qt->ch);
		if(pch==NULL)return FALSE;
		/*-----------------------------------------
		*  �����ǿ�Ƴ�磬���ڵ����ֵ�����Ӧ�öϵ�
		*  ����������ǲ�
		------------------------------------------*/
		if(!is_force_charge_on(ch))
			reset_out5v();           //���϶ϵ�
	}
  return TRUE;
}
/*����һ�����*/
BOOL request_charge_hangup(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	Queue_Type *qt = request_channel_find(ch);
	if(pch==NULL)return FALSE;
	if(qt ==NULL)return FALSE;
	if(!inited)request_init();
	qt->used=0;
	reset_out5v();           //���϶ϵ�
	return TRUE;
}

/*�ָ�һ�����*/
BOOL request_charge_recovery(U8 ch)
{
	Channel*pch = channel_data_get(ch);
	Queue_Type *qt = request_channel_find(ch);
	if(pch==NULL)return FALSE;
	if(qt ==NULL)return FALSE;
	if(!inited)request_init();
	qt->used=1;
	return TRUE;
}

/*������*/
BOOL request_charge_hangup_all(U32 seconds)
{
	if(!inited)request_init();
	{
		int ch;
		for(ch=1;ch<=CHANNEL_MAX;ch++)
		{
			Channel*pch = channel_data_get(ch);
			if(pch==NULL)continue;
			//if(!is_force_charge_on(ch))//��ǿ�Ƴ��,�Ͷϵ�
			 reset_out5v();//��ֹ�������
		}
		hangall=TRUE;
		hangtime=seconds; //���𣬲�����ʱ
		return TRUE;
	}
}

/*���������Ƿ����?*/
BOOL ld_is_queue_hang(void)
{
	//����󣬰�ǿ�Ƴ������
	return (BOOL)((system.enable==0)||((system.enable==1)&&(system.mode==1)));
}
/*===================================================
                ����������
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(queue)
{
	PROCESS_BEGIN();
	while(1)
	{
		//���Գ��
		if(system.enable==1)
		{
			//ǿ�Ƴ�緽ʽ
		  if(system.mode==1)
			{
				int i = 0;
				for(;i<CHANNEL_MAX;i++)
				{
					direct_charge(i+1,(BOOL)system.chs[i]);//ǿ�����
				}
			}	
			//���ɳ�緽ʽ
			else{
			bubble_sort();          //����
			charge_timeout();			  //��ʱ
			charge_front();         //ǰ���ȳ�		
		 	}
		}
		
		//�����
		else{
		  request_charge_hangup_all(0);//ǿ�ƹض�
		}
 	  os_delay(queue,100);   
	}
	PROCESS_END();
}

