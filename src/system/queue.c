
/*===================================================
����:����Ŷ��㷨
�㷨����:

�ֵ�����: A.����Ϊ0�ֵ�    B.7Сʱ�ڳ��   C.�����ɿ��ܲ���

          1.���ȼ� A�� > B�� >C��
					2.���A�����ʱ���ȵ��ȵã��Ŷ�
					3.���B�����ʱ�������������Ŷӣ�
					4.
====================================================*/
#include "lib.h"
#include "channel.h"
#include "string.h"


/*���ֵ���*/
#define MAX_CHANNEL           5

/*����ͬʱ�������ֵ���*/
#define MAX_CHANNEL_CHARGE    2

/*����(contiki����ռ,���Բ�����߳�ͬʱ����)*/
#define queue_lock()
#define queue_unlock()


/*�Ŷӽṹ*/
typedef struct{
  U8 \
	used:1,    //�Ѿ�ʹ��:1   ��δʹ��:0
	charge:1,  //������:1   ������:0
	reversed:6;//δʹ��λ
	Channel*ch;
}Queue_Type;
static Queue_Type list[MAX_CHANNEL];    //�б�
/*===================================================
                ���غ���
====================================================*/
//��ʼ��
static void queue_init(void)
{
	memset(ready,0,sizeof(ready));
	memset(urgent,0,sizeof(urgent));
	memset(full,0,sizeof(full));
}
//����һ���б�
static BOOL queue_insert(Queue_Type*qt,Channel*ch)
{
	int i = 0;
	queue_lock();
	for(;i<MAX_CHANNEL;i++){
		if(qt[i].used==0){
			memset(&qt[i],0,sizeof(Queue_Type));
			qt[i].ch=ch;
			queue_unlock();
			return TRUE;
		}
	}
	queue_unlock();
	return FALSE;
}

//��һ���б�ɾ��
static BOOL quque_remove(Queue_Type*qt,Channel*ch)
{
  int i = 0;
	queue_lock();
	for(;i<MAX_CHANNEL;i++){
		if(qt[i].used==1&&qt[i].ch==ch){
			memset(&qt[i],0,sizeof(Queue_Type));
			queue_unlock();
			return TRUE;
		}
	}
	queue_unlock();
	return FALSE;
}

/*�����б��еĲֵ�*/
static Queue_Type*where_is_queue(Channel*ch)
{
	int i = 0;
	Queue_Type*qt=NULL;
	queue_lock();
	for(;i<MAX_CHANNEL;i++){
		if(ready[i].used==1&&ready[i].ch==ch){qt=&ready[i];goto WHERE_END;}
		if(urgent[i].used==1&&urgent[i].ch==ch){qt=&urgent[i];goto WHERE_END;}
		if(full[i].used==1&&full[i].ch==ch){qt=&full[i];goto WHERE_END;}
	}
			
	WHERE_END:
	queue_lock();
	return qt;
}
  

/*�Ŷӵ����߳�*/
#include "contiki.h"
static struct etimer et_queue;
PROCESS(thread_queue, "������");
AUTOSTART_PROCESSES(thread_queue);
PROCESS_THREAD(thread_queue, ev, data)  
{
	//��ǰ���ڳ��ĸ���
	static U8 charge_counter;                
	PROCESS_BEGIN();
	while(1)
	{
		
		//1.�����б�,�����ȳ�
		
		//2.�����б�,ð������,��������ȳ�
		
		//3.����б�,ð������,����С���ȳ�
		
		//��ʱ10ms
    os_delay(et_queue,10);
	}

	PROCESS_END();
}
/*===================================================
                ȫ�ֺ���
====================================================*/
/*ע��һ���ֵ�����Ҫ���
* ����: TRUE:ע��ɹ�
*     : FALSE:ע��ʧ��
*/
BOOL queue_regist(Channel*ch)
{
  //����==0 ����:�����б�
	//������,����:����б�
	//����:��������б� 
	if(ch->Ufsoc==0)
		return queue_insert(urgent,ch);
	if(ch->cs.full_charge==1)
		return queue_insert(full,ch);
	else
		return queue_insert(ready,ch);
}

/*
* ��ѯ�ֵ��Ƿ���Գ��
* ����:-1:����   0:����  1:���Գ��
*/
int queue_isok(Channel*ch)
{
  Queue_Type*qt = where_is_queue(ch);
	if(qt==NULL)return -1;
	return qt->charge;
}

/* 
* ɾ��ע���еĲֵ�:�Ҳ�������
* ���أ�TRUE :�ɹ�ɾ�� 
*     ��FALSE:ɾ��ʧ��
*/
BOOL queue_delete(Channel*ch)
{
  Queue_Type *qt = where_is_queue(ch);
	queue_lock();
	if(qt==NULL){queue_unlock();return FALSE;}
	memset(qt,0,sizeof(Queue_Type));
	queue_unlock();
	return TRUE;
}
