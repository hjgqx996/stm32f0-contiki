#include "types.h"
#include "string.h"


/*===================================================
                上行协议通讯数据包操作函数
====================================================*/

typedef struct{
	//帧头     地址  命令    长度(高字节在前)  数据
	U8 AA ,BB ,addr  ,cmd   ,len              ,data[1];
}packet;

//如果支持多通道数据包接收，必须使用私有数据
typedef struct{
  //接收长度   状态
	int size    ,state;
  //有效数据包
	packet p;
}packetx;


packetx *packet_recv(U8 data,packetx*p)
{
//  U8*d=(U8*)&(p->p);
//  if(p->s

//	DecodeError:
	
}