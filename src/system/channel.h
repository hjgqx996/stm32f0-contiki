
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
	baibi:1,         //�ڱۿ��ع���
	ir:1,            //����ʶ�����
	daowei:1,        //��λ���ع���
	temp:1,          //���籦����  temp<0 || temp>65
	thimble:1,       //����ʶ�����
	reversed:1;      //����
}ChannelError;


/*��籦�����־*/
typedef enum{
	BAO_OUTPUT_NULL= 0x00,
	BAO_ALLOW   = 0x05,  					//��籦�������
	BAO_NOTALLOW= 0x06,  					//��籦���������
	BAO_ALLOW_ONE_HOUR = 0x07,  	//�������1Сʱ
}BaoOutput;

/*�ֵ����ݽṹ*/
typedef struct{	
	/*--------------���ýӿ�--------------------------*/
	ChannelConfigureMap*map;        //ͨ������io����
	U8 addr;                        //ͨ����ַflash[]<----System.addr_ch-------Channel.addr

	/*--------------�쳣����--------------------------*/

	/*--------------�ֵ���----------------------------*/
	BOOL flash;                     //�Ƿ���˸
	volatile int  flash_ms;         //��˸��ʱ��(ms)
	volatile int  flash_now;        //��ʱ
	
	/*--------------iic�����л�------------------------*/
	U8  iic_dir;                     //iic���� 0:��������  1:����ת
	U8  iic_dir_counter;             //�������
	int dingzhen_counter;            //����ʶ����� ����
	int ir_error_counter;            //����ʶ����� ����
	
	/*--------------iic,ir�л�------------------------*/
	U8 iic_ir_mode;                 //iic���� 0:��������  1:����ת
	U8 iic_ir_mode_counter;         //�������

	/*--------------����״̬����----------------------*/
	ChannelState state;             //����״̬
	ChannelWarn  warn;              //���и澯
	ChannelError error;             //���д���
		
	/*-------------ͨ������-----------------------------iic-----ir-*/
	volatile U8  Ufsoc;             //ʣ�����  %        ��     ��
	volatile U16 Voltage;           //��ѹ,��λ mV       ��     ��
	volatile U16 RemainingCapacity; //ʣ������ mAh       ��     ��
	volatile U16 CycleCount;        //���ѭ������       ��     ��
	volatile S16 AverageCurrent;    //���ƽ������  mA   ��     ��
	volatile S8 Temperature;        //��о�¶�	    ��   ��     ��
	volatile U8 Ver;                //�����籦�汾            ��

	U8 id[10];                      //��籦���
	BaoOutput bao_output;           //��籦�����־ 

	/*--------------�Ƿ�������------------------------*/
	U8 readok;                      //��id,�����ݣ��Ƿ�����,����>=2����
	S8 readerr;                     //���������
	BOOL first_insert;              //��һ�β���(==TRUE,ʱ��ѭ��������thread_insert�̶߳�)
	
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
/*-----------------------------------------------------
* channel ���ݳ�ʼ�� ��0����ַ����
-------------------------------------------------------*/
BOOL channel_data_init(void);               //��ʼ�� 
BOOL channel_data_clear_by_addr(U8 ch_addr);//������
BOOL channel_data_clear(U8 ch);             //������
void channel_addr_set(U8*addrs);            //���òֵ���ַ
BOOL channel_clear(U8 ch);
/*-----------------------------------------------------
* channel ���ݻ�ȡ  ����
-------------------------------------------------------*/
Channel*channel_data_get(U8 channel);
Channel*channel_data_get_by_addr(U8 addr);
int channel_data_get_index(Channel*ch);

/*-----------------------------------------------------
* channel ����
-------------------------------------------------------*/
void channel_led_flash(U8 ch,U8 seconds);
void channels_les_flash_timer(int timer_ms);

/*------------------------------------------------------
�ֵ�״̬���澯������ 
-------------------------------------------------------*/
void channel_check_timer_2s(void);

/*------------------------------------------------------
		�ж�
-------------------------------------------------------*/
#define isvalid_daowe()  (ld_gpio_get(pch->map->io_detect)) //��λ������Ч
#define isvalid_baibi()  (ld_gpio_get(pch->map->io_sw))     //�ڱۿ�����Ч
#define isin5v()         (ld_gpio_get(pch->map->io_mp_detect))//�Ƿ�������
#define isout5v()        (ld_gpio_get(pch->map->io_mp))     //�Ƿ������
#define is_ver_6()       ((pch->id[6]&0x0F)==0x06)        //6����
#define is_ver_7()       ((pch->id[6]&0x0F)==0x07)        //7����
#define is_ver_lte_5()   ((pch->id[6]&0x0F)<=0x05)        //5��������
#define is_readok()      (pch->state.read_ok==1)             //�ж϶�ȡ�ɹ�
#define is_readerr()     (pch->state.read_error==1)          //�ж��Ƿ��ʧ��
#define is_has_read()    ( (pch->state.read_ok==1) || (pch->state.read_error==1)) //�ж��Ƿ��Ѿ���
#define set_out5v()     ld_gpio_set(pch->map->io_mp,1) //���5V
#define reset_out5v()   ld_gpio_set(pch->map->io_mp,0) //�����5V

int channel_read(Channel*pch,READ_TYPE_CMD cmd,U8*dataout,int ms_timeout,BOOL once);
void channel_save_data(Channel*ch,U8*data);
#endif



