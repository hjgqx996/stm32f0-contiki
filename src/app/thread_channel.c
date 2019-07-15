#include "contiki.h"
#include "types.h"


/*===================================================
                �ֵ�����
====================================================*/

static struct etimer et_channel;
PROCESS(channel_thread, "ͨ������");
PROCESS_THREAD(channel_thread, ev, data)  
{
	static U32 timeout = 0;
	BOOL flag=FALSE;
	PROCESS_BEGIN();
	while(1)
	{
		U8 i = 0;
		
		//��ʱ2.8��
		if(timeout==0)timeout=time(0)+2800;
		if(!((time(0)-timeout)>0x80000000)){
			flag=TRUE;
			timeout=time(0)+2800;
		}
		
		for(;i<5;i++){
			//ˢ�²ֵ� ״̬���澯���쳣
			channel_state_check(i);
			channel_warn_check(i);
			channel_error_check(i);
	
			//����ֵ��б�����ȡ�ֵ�����.Ƶ��2.8��
			if(flag)
			{
				channel_read_data(i);
			}
		}
		flag=FALSE;
		os_delay(et_channel,10);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(channel_thread);