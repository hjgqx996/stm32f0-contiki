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
			{}
		}
		flag=FALSE;
		os_delay(et_channel,10);
	}

	PROCESS_END();
}



AUTOSTART_PROCESSES(channel_thread);

/*===================================================
                iic-�ֵ�����
====================================================*/
PROCESS(thread_iic, "iic����");
PROCESS_THREAD(thread_iic, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		//�ȴ����Ѷ�ȡiic��Ϣ
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(thread_iic);

/*===================================================
                ����-�ֵ�����
====================================================*/
PROCESS(thread_ir, "ir����");
PROCESS_THREAD(thread_ir, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		//�ȴ����Ѷ�ȡir
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(thread_ir);



