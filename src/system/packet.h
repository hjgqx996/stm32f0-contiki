#ifndef __PACKET_H__
#define __PACKET_H__
#include "types.h"


//������ݰ�����
#define PACKET_DATA_MAX     400

/*ͨѶЭ��������*/
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
	PC_HANDSHAKE=0x16,      //����
	
	//========================================
	PC_DEBUGINFO=0x55,      //��չָ�������Ϣ
}PACKET_CMD;

/*����һ������˽�б���*/
typedef struct{
int len    ,state;
}PacketState;

/*���ݰ��ṹ*/
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


/*֧�ֶ�ͨ�����ݽ���*/
extern HPacket *packet_recv(U8 data,HPacket*m);            /*����һ���ֽڣ������Ƿ�ɹ�����һ�����ݰ�*/
extern int packet_send(HPacket*hp, U8 cmd,U16 len,U8*data,U8 addr);/*����һ���������ط��ͳ���*/
#endif
