#ifndef __PACKET_H__
#define __PACKET_H__
#include "types.h"


//最大数据包长度
#define PACKET_DATA_MAX     400

/*通讯协议命令码*/
typedef enum{
	
	PC_HEART_BREAK = 0x01,  //心跳
	PC_LEASE = 0x02,        //租借
  PC_RETURN= 0x03,        //归还
	PC_CTRL  = 0x04,        //控制
	
	//========================================
	PC_UPDATE_ENTRY= 0x05,  //进入固件更新
	PC_UPDATE_MODE=0x06,    //查询固件更新模式
	PC_UPDATE_DATA=0x07,    //固件数据
	//========================================
	PC_HANDSHAKE=0x16,      //握手
	
	//========================================
	PC_DEBUGINFO=0x55,      //扩展指令，调试信息
}PACKET_CMD;

/*接收一个包的私有变量*/
typedef struct{
int len    ,state;
}PacketState;

/*数据包结构*/
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


/*支持多通道数据接收*/
extern HPacket *packet_recv(U8 data,HPacket*m);            /*接收一个字节，返回是否成功接收一个数据包*/
extern int packet_send(HPacket*hp, U8 cmd,U16 len,U8*data,U8 addr);/*发送一个包，返回发送长度*/
#endif
