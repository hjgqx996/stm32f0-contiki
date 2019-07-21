#include "includes.h"
/*===================================================
                上行协议通讯数据包操作函数
====================================================*/
/*接收一个字节，返回是否成功接收一个数据包*/
HPacket *packet_recv(U8 data,HPacket*m)
{
	  PacketState*s = &m->s;
    unsigned char*d = (unsigned char*)&m->p;
    if(m->s.len > PACKET_DATA_MAX)goto DECODE_ERROR;
    d[m->s.len] = data;
    m->s.len++;
    switch(m->s.state)
    {
			case 0:
							if(data==0xAA){m->s.state++;break;}
							else 
							goto DECODE_ERROR;
			case 1:
							if(data==0xBB){m->s.state++;break;}
							else 
							goto DECODE_ERROR;
			//add,cmd,h,l
			case 2:
			case 3:
			case 4:
			case 5:
							m->s.state++;
							break;       
			//接收数据
			case 6:
			{
				U16 datalen = 0;
				datalen = m->p.hlen;
				datalen<<=8;
				datalen|=m->p.llen;
				if(m->s.len <= (datalen+6))break;
				else{		
					if(cs8(d,m->s.len-1)==data)
					{
						//接收到一个数据包
						return m;
					}else goto DECODE_ERROR;
				}
			}
			default:
							goto DECODE_ERROR;
    }
    return NULL;
    DECODE_ERROR:
    memset(s,0,sizeof(PacketState));
    return NULL;
}
/*===================================================
           发送数据
====================================================*/
int packet_send(HPacket*hp, U8 cmd,U16 len,U8*data,U8 addr)
{
	packet *p = &hp->p;
	if(hp==NULL)return FALSE;
	//memset(hp,0,sizeof(HPacket));
	p->AA=0xAA;
	p->BB=0xBB;
	p->addr=addr;
	p->cmd=cmd;
	p->hlen=len>>8;
	p->llen=len&0xFF;
	if(len>0&&p->data!=data)
		memcpy(p->data,data,len);
	if(len>(PACKET_DATA_MAX-7))return FALSE;
	p->data[len] = cs8((U8*)p,len+6);
	enable_485_tx();//使能发送
	ld_uart_send(COM_485,(U8*)p,len+7);
	return len;
}




