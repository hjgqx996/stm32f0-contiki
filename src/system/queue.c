
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
/*�Ŷӵ����߳�*/
AUTOSTART_THREAD_WITH_TIMEOUT(queue)
{
	//��ǰ���ڳ��ĸ���
	static U8 charge_counter;   
  memset(list,0,sizeof(list));	
	PROCESS_BEGIN();
	while(1)
	{
		
    os_delay(queue,10);
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

}

/*
* ��ѯ�ֵ��Ƿ���Գ��
* ����:-1:����   0:����  1:���Գ��
*/
int queue_isok(Channel*ch)
{

}

/* 
* ɾ��ע���еĲֵ�:�Ҳ�������
* ���أ�TRUE :�ɹ�ɾ�� 
*     ��FALSE:ɾ��ʧ��
*/
BOOL queue_delete(Channel*ch)
{

}
