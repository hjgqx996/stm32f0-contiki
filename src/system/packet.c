#include "includes.h"
/*===================================================
                ����Э��ͨѶ���ݰ���������
====================================================*/
/*����һ���ֽڣ������Ƿ�ɹ�����һ�����ݰ�*/
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
			//��������
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
						//���յ�һ�����ݰ�
						memset(&(m->s),0,sizeof(PacketState));
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
           ��������
====================================================*/
int packet_send(HPacket*hp, U8 cmd,U16 len,U8*data,U8 addr)
{
	U8 cs = 0;
	packet *p = &hp->p;
	if(hp==NULL)return FALSE;
	//memset(hp,0,sizeof(HPacket));
	p->AA=0xAA;
	p->BB=0xBB;
	p->addr=addr;
	p->cmd=cmd;
	p->hlen=len>>8;
	p->llen=len&0xFF;
	
	if(len>0&& ((int)data) != ((int)hp->p.data) )
		memcpy(hp->p.data,data,len);
	cs= cs8((U8*)p,6+len);	
	hp->p.data[len] = cs;
	enable_485_tx();//ʹ�ܷ���
	delayms(1);
	ld_uart_send(COM_485,(U8*)&hp->p,7+len);
	delayms(10);
	return len;
}




