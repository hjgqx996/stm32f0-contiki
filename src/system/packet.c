#include "types.h"
#include "string.h"


/*===================================================
                ����Э��ͨѶ���ݰ���������
====================================================*/

typedef struct{
	//֡ͷ     ��ַ  ����    ����(���ֽ���ǰ)  ����
	U8 AA ,BB ,addr  ,cmd   ,len              ,data[1];
}packet;

//���֧�ֶ�ͨ�����ݰ����գ�����ʹ��˽������
typedef struct{
  //���ճ���   ״̬
	int size    ,state;
  //��Ч���ݰ�
	packet p;
}packetx;


packetx *packet_recv(U8 data,packetx*p)
{
//  U8*d=(U8*)&(p->p);
//  if(p->s

//	DecodeError:
	
}