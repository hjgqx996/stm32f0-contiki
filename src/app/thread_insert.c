#include "contiki.h"
#include "types.h"

/*===================================================
                ���غ���
====================================================*/
/*�����磬�������Լ�״̬*/
#define hang_up_until_charge(ch,state) 

/*��⵽��磬�ָ������״̬*/
#define recovery_state(ch)  




/*��籦����ͨ��״̬��*/
static void charge_fsm(U8 channel,void*arg)
{

	//channel����
	FSM*fsm;
	Start(����籦����){
		//�������:     ���ɹ�->ʶ���籦
				
	}
	
	State(ʶ���籦)
	{
    //��ʶ��-->ֹͣ���
		//����ʶ��->���5��
	}
	
	State(���5��)
	{
	
		//������
		
		//�ȴ�5���ֹͣ���
		
		//��һ��ʶ���籦:��ʶ��->�����Ƿ�Ϊ0    ����ʶ��:������籦,��λ����->����籦����
	
	}
	
	State(�����Ƿ�Ϊ0)
	{
		//����<=0-->������
		
		//���� >0-->ֹͣ���
		
	}
	
	State(���10����)
	{
	  //������
		
		//���� >0 �� ��ʱ10����-->ֹͣ���

	}
	
	State(ֹͣ���)
	{
	  //�Ŷӳ��
		goto ���7Сʱ;
	}
	State(���7Сʱ)
	{
	  //����<200mA,����120��->�������
		
		//�Ѿ����7Сʱ->�������
	}
	State(�������)
	{
	  //����>95%:������
		//����>=85%,<=95%:1Сʱ����1�Σ�3�λ���
		//����<85%,3Сʱ����һ�Σ����޻���
	
	}
	
	/*
	*  ��һ�����ڳ��Ĳֵ����ŶӴ�Ϻ󣬽������״̬
	*  ������������Ŷ�ʱ���������״̬
	*  ������ϻָ��󣬿��Իָ���֮ǰ��״̬
	*/
	State(����Ŷ�)
	{
	
	
	}
	
  default:{}}
		
		
	/*�����ʱ����ռ��״̬�����𵽳���Ŷ�*/
  		
		
}




/*===================================================
                ȫ�ֺ���
====================================================*/
const char*vd="kaihua.yang";
static process_event_t pet={0xF5};
static struct etimer et_insert;
PROCESS(insert_thread, "��������");
PROCESS_THREAD(insert_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		PROCESS_NAME(comm_thread);
		pet++;
    process_post(&comm_thread,pet,(void*)vd);
		os_delay(et_insert,10);
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(insert_thread);