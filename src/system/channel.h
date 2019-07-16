
#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include "types.h"
/*===================================================
                �꣬����
====================================================*/
#pragma pack(1)

/*�ֵ��ӿ�����*/
typedef const struct{
	U8 io_motor;       //��ŷ�io��
	U8 io_sw;          //�ڱۿ���io��
  U8 io_detect;      //��λ����io��
	
  U8 io_ir;          //���ⷢ��io��
  U8 io_re;          //�������io��
	
  U8 io_led;         //led��io��
  U8 io_mp;          //���ʹ��io��
  U8 io_mp_detect;   //���������io��
	
	U8 io_sda;         //iic sda��
	U8 io_scl;         //iic scl��
}ChannelConfigureMap;


/*�ֵ�״̬���ݽṹ:��λ��ǰ*/
typedef struct{
	U8 \
	read_error:1,    //�б������Ƕ�ȡ����
	read_ok:1,       //�б������Ƕ�ȡ��ȷ
  reversed:3,      //����
	read_from_ir:1,  //ʹ�ú����ȡ
	full_charge:1,   //�����
	charging:1;      //�б������ݶ�ȡ���������ڳ��
}ChannelState;

/*�ֵ��������ݽṹ:��λ��ǰ*/
typedef struct{
	U8 \
	temperature:1,   //�¶ȱ��� <0 || >60
	eject:1,         //�쳣�����¼�������־20���Ӻ���0
  mp:1,            //5V����������(���������ⲻ����� �� ���������⵽���)
	reversed:1,      //����
	restart:1,       //ģ���쳣��������
	reversed2:3;      //����
}ChannelWarn;

/*�ֵ��������ݽṹ:��λ��ǰ*/
typedef struct{
	U8 \
	lease:1,         //������
	motor:1,         //��ŷ�/�������
  sw:1,            //�ڱۿ��ع���
	ir:1,            //����ʶ�����
	key:1,           //��λ���ع���
	bao:1,           //���籦����  temp<0 || temp>65
	thimble:1,       //����ʶ�����
	reversed:1;      //����
}ChannelError;


/*��籦�����־*/
typedef enum{
	BAO_ALLOW   = 0x05,  //��籦�������
	BAO_NOTALLOW= 0x06,  //��籦���������
	BAO_ALLOW_X = 0x07,  //??????
}BaoOutput;

/*�ֵ����ݽṹ*/
typedef struct{
	/*-------------ͨ������--------------------------*/
	volatile U8  Ufsoc;             //ʣ�����  %
	volatile U16 Voltage;           //��ѹ,��λ mV
	volatile U16 RemainingCapacity; //ʣ������ mAh
	volatile U16 CycleCount;        //���ѭ������
	volatile S16 AverageCurrent;    //���ƽ������  mA
  volatile S8 Temperature;        //��о�¶�	    ��
	volatile U8 Ver;                //�����籦�汾
	
	U8 id[10];                      //��籦���
	U8 addr;                        //ͨ����ַflash[]<----System.addr_ch-------Channel.addr
	BaoOutput bao_output;           //��籦�����־ 
	
	/*--------------���ýӿ�--------------------------*/
	ChannelConfigureMap*map;        //ͨ������io����
	
	/*--------------����״̬����----------------------*/
	U8 iic_dir;                     //iic���� 0:��������  1:����ת
	ChannelState state;             //����״̬
	ChannelWarn  warn;              //���и澯
	ChannelError error;             //���д���
	
	S32 priority;                   //���������ȼ�-->������ʹ��(���ȼ�+ð��)�Ŷӳ��
	
	FSM insert;                     //��籦����ֵ�״̬������:˽��
}Channel;

#pragma pack()

/*===================================================
                ȫ�ֺ���
====================================================*/
/*��ȡ�ֵ�����
*channel:1-n
*/
Channel*channel_data_get(U8 channel);
#endif



