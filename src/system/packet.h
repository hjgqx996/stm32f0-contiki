#ifndef __PACKET_H__
#define __PACKET_H__
#include "types.h"


//������ݰ�����
#define PACKET_DATA_MAX     1024


typedef struct{
int len    ,state;
}PacketState;
typedef struct{
	//֡ͷ     ��ַ  ����    ����(���ֽ���ǰ)  ����
	U8 AA ,BB ,addr  ,cmd   ,hlen,llen         ,data[PACKET_DATA_MAX-6];
}packet;

//���֧�ֶ�ͨ�����ݰ����գ�����ʹ��˽������
typedef struct{
  //���ճ���   ״̬
	PacketState s;
  //��Ч���ݰ�
	packet p;
}HPacket;


/*����һ���ֽڣ������Ƿ�ɹ�����һ�����ݰ�*/
extern HPacket *packet_recv(U8 data,HPacket*m);

#endif
