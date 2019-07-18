
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
	BAO_ALLOW_ONE_HOUR = 0x07,  //�������1Сʱ
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
	
	/*--------------iic�����л�------------------------*/
	U8  iic_dir;                     //iic���� 0:��������  1:����ת
	U8  iic_dir_counter;             //�������
	
	/*--------------iic,ir�л�------------------------*/
	U8  iic_ir_mode;                 //iic���� 0:��������  1:����ת
	int iic_ir_mode_counter;         //�������
	
	/*--------------����״̬����----------------------*/
	ChannelState state;             //����״̬
	ChannelWarn  warn;              //���и澯
	ChannelError error;             //���д���
	
	/*--------------�쳣����--------------------------*/
	
	/*--------------�ֵ���----------------------------*/
	BOOL flash;                     //�Ƿ���˸
	int  flash_ms;                  //��˸��ʱ��(ms)
	int  flash_now;                 //��ʱ
	
	FSM insert;                     //��籦����ֵ�״̬������:˽��
}Channel;

#pragma pack()

/*===================================================
                 ��籦ͳһ����iic������
====================================================*/
/*  ���� */
typedef enum{
	RC_READ_ID   =10,      //:�����ID
	RC_READ_DATA =20,      //:������
	RC_UNLOCK    =30,      //:����05
	RC_LOCK      =50,      //:����06
	RC_UNLOCK_1HOUR =40,   //:����1Сʱ07
	RC_OUTPUT,             //:�������־
}READ_TYPE_CMD;          //��籦����
/*  ͨѶ��ʽ */
typedef enum{
	RTM_IIC=0,
	RTM_IR=1,
}READ_TYPE_MODE;

/*===================================================
                ȫ�ֺ���
====================================================*/
BOOL channel_id_is_not_null(U8*id);
/*
* channel���ݳ�ʼ��
*/
BOOL channel_data_init(void);
BOOL channel_data_clear_by_addr(U8 ch_addr);//������

/*��ȡ�ֵ�����
*channel:1-n
*/
Channel*channel_data_get(U8 channel);

/*��ȡ�ֵ�����--by addr
*channel:1-n
*/
Channel*channel_data_get_by_addr(U8 addr);


/*ͨ������˸����*/
void channel_led_flash(U8 ch,U8 seconds);
void channels_les_flash_timer(int timer_ms);

/*----------------------------------
��籦�����ض���:�Ƿ�æ,�� ,�Ƿ����
-----------------------------------*/
BOOL channel_read_busy(U8 ch,READ_TYPE_MODE mode);
BOOL channel_read_start(U8 ch,READ_TYPE_MODE mode,BOOL opposite,READ_TYPE_CMD cmd);
int channel_read_end(U8 ch,READ_TYPE_MODE mode,U8*dataout);

#endif



