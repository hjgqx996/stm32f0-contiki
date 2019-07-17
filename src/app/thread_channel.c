#include "includes.h"

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
  channel_data_init();//��ʼ���ֵ�����
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
                ��籦������
====================================================*/
static struct etimer et_read;
PROCESS(thread_read, "��籦������");
AUTOSTART_PROCESSES(thread_read);
PROCESS_THREAD(thread_read, ev, data)  
{
	PROCESS_BEGIN();
	while(1)
	{
		PROCESS_WAIT_EVENT();
		switch(ev)
		{
			case PROCESS_EVENT_READ_ID:
				break;
			case PROCESS_EVENT_READ_DATA:
				break;
			case PROCESS_EVENT_READ_UNLOCK:
				break;
			case PROCESS_EVENT_READ_
		
		}
		//�ȴ����Ѷ�ȡiic��Ϣ
		//ld_bq27541_read_id(21,22,ids);
		//ld_bq27541_check_ack(22,21);
	}

	PROCESS_END();
}
