

#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "contiki.h"
//===========================����=============================//
//��ȡ��籦-�̼߳�ͨѶ��
typedef struct{
	struct process* master;//�����߳�
  
}ProcessData_iic_ir;

typedef enum{
	PROCESS_EVENT_PACKET           =0x40,   //�¼�:���ݰ�����
	PROCESS_EVENT_COMM_LEASE,               //�¼�:������ݰ�����
	PROCESS_EVENT_READ_ID,                  //�¼�:�����ID
	PROCESS_EVENT_READ_DATA,                //�¼�:������
	PROCESS_EVENT_READ_UNLOCK,              //�¼�:����05
	PROCESS_EVENT_READ_LOCK,                //�¼�:����06
	PROCESS_EVENT_READ_UNLOCK_1HOUR,        //�¼�:����1Сʱ07
	PROCESS_EVENT_READ_OUTPUT,              //�¼�:�������־
}PROCESS_EVENT;


//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//<o>485���ں�  <1=>uart1 <2=>uart2
#define COM_485                   2

//485����ʹ��
#define enable_485_tx()   ld_gpio_set(2,1)
#define disable_485_tx()  ld_gpio_set(2,0)
#define enable_485_rx()   disable_485_tx()
#define disable_485_rx()  enable_485_tx()

//<o>�ֵ�����
#define CHANNEL_MAX                5

//<o>�ֵ���ų���
#define CHANNEL_ID_MAX             10

//<o>Ӳ�����<0x0-0xFFFF:4>
#define HARDWARE_VERSION          0x0202
//<o>������<0x0-0xFFFF:4>
#define SOFTWARE_VERSION          0x0001

//<o>���ʱ���ֵ�����˸Ƶ��ms
#define LEASE_LED_FLASH_TIME      200

//<o>�ֵ�����ʱ���������С����%
#define CHANNEL_LED_LIGHT_UFSOC   50

//�ж��Ƿ���6����
#define is_6_battery(ids)          if((ids[6]&0x0F)==0x06)              
//��ŷ����
#define dian_ci_fa(channel,level)  ld_gpio_set(channel->map->io_motor,level)


/******************��账����***************************/
#define Lease_fall   					0x00
#define Lease_success   			0x01
#define Lease_differ  				0x02
#define Lease_decrypt_fall    0x03
#define Lease_dianchifa_fall  0x04
/*********************************************************/

/******************�黹������***************************/
#define Return_fall   					0x00
#define Return_success   			  0x01
#define Return_unrecognized   	0x02
#define Return_timeout   	      0x03
/*********************************************************/

/******************����������***************************/
#define Cmd_fall   					  0x00
#define Cmd_success   			  0x01
/*********************************************************/

/******************��������*******************************/
#define IR_GETID          	10   //��ȡID
#define IR_GETDATA          20   //��ȡ����
#define IR_OUTENABLE       	30   //�������
#define IR_OUTDISABLE      	50   //��ֹ���
#define IR_OUTANHOUR      	40   //���1Сʱ



//==========================================================//
#include "channel.h"
extern ChannelConfigureMap channel_config_map[];




#endif