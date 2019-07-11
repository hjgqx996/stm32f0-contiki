
#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include "types.h"
/*===================================================
                �꣬����
====================================================*/
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
	/*--------------���ýӿ�--------------------------*/
	ChannelConfigureMap*map;        //ͨ������io����
	
	/*--------------����״̬����----------------------*/
	U8 iic_dir;                     //iic���� 0:��������  1:����ת
	ChannelState cs;                //����״̬
	ChannelWarn  cw;                //���и澯
	ChannelError ce;                //���д���
	
	S32 priority;                   //���������ȼ�-->������ʹ��(���ȼ�+ð��)�Ŷӳ��
	
	FSM insert;                     //��籦����ֵ�״̬������:˽��
}Channel;


/*===================================================
                ȫ�ֺ���
====================================================*/

#endif



