#ifndef __PACKET_H__
#define __PACKET_H__
#include "types.h"


//最大数据包长度
#define PACKET_DATA_MAX     1024


typedef struct{
int len    ,state;
}PacketState;
typedef struct{
	//帧头     地址  命令    长度(高字节在前)  数据
	U8 AA ,BB ,addr  ,cmd   ,hlen,llen         ,data[PACKET_DATA_MAX-6];
}packet;

//如果支持多通道数据包接收，必须使用私有数据
typedef struct{
  //接收长度   状态
	PacketState s;
  //有效数据包
	packet p;
}HPacket;


/*接收一个字节，返回是否成功接收一个数据包*/
extern HPacket *packet_recv(U8 data,HPacket*m);

#endif
