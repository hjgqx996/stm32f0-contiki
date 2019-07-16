#ifndef __PACKET_H__
#define __PACKET_H__
#include "types.h"


//������ݰ�����
#define PACKET_DATA_MAX     1024

typedef enum{
	
	PC_HEART_BREAK = 0x01,  //����
	PC_LEASE = 0x02,        //���
  PC_RETURN= 0x03,        //�黹
	PC_CTRL  = 0x04,        //����
	
	//========================================
	PC_UPDATE_ENTRY= 0x05,  //����̼�����
	PC_UPDATE_MODE=0x06,    //��ѯ�̼�����ģʽ
	PC_UPDATE_DATA=0x07,    //�̼�����
	//========================================
}PACKET_CMD;






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

/*����һ���������ط��ͳ���*/
int packet_send(HPacket*hp, U8 cmd,U16 len,U8*data);
#endif
