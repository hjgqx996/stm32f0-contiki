#include "contiki.h"
#include "types.h"


/*===================================================
                ���غ���
====================================================*/


/*===================================================
                ȫ�ֺ���
====================================================*/

static struct etimer et_channel;
PROCESS(channel_thread, "ͨ������");
PROCESS_THREAD(channel_thread, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
     //��籦�Ƿ���Ч
		
		 //��һ�γ�籦����
		
		 //ת����籦����
		
		 //ͨ��״̬,�澯��������
		
		 //��ʱ10ms
		 os_delay(et_channel,10);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(channel_thread);