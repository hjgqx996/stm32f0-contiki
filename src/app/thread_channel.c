#include "contiki.h"
#include "thread_channel.h"


/*===================================================
                ���غ���
====================================================*/


/*===================================================
                ȫ�ֺ���
====================================================*/
PROCESS(ir_thread, "��������");
PROCESS_THREAD(ir_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    //���ⷽʽ��ȡ��籦����
	}

	PROCESS_END();
}


PROCESS(iic_thread, "iic����");
PROCESS_THREAD(iic_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
    //iic��ʽ��ȡ��籦����
	}

	PROCESS_END();
}



static struct etimer et_channel;
PROCESS(channel_thread, "ͨ������");
PROCESS_THREAD(channel_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		 etimer_set(&et_channel, CLOCK_SECOND);              // etimer���ʱ��Ϊ5s
     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_channel));  // �ȴ���ʱ�����
     //��籦�Ƿ���Ч
		
		 //��һ�γ�籦����
		
		 //ת����籦����
	}

	PROCESS_END();
}

AUTOSTART_PROCESSES(channel_thread);