#include "includes.h"
unsigned int idle_count = 0; 

/*�Զ������̵߳�ַ*/
extern const  U32 autostart$$Base;
extern const  U32 autostart$$Limit;

int main(void)
{
	channel_data_init();//��ʼ���ֵ�����
	ld_system_init();   //ϵͳ����  
	ld_dev_init();      //�ײ��ʼ��
	clock_init();       //ϵͳ����ʱ��
	process_init();     //�̳߳�ʼ��
	process_start(&etimer_process,NULL); //������ʱ��
	autostart_start((struct process * const*)&autostart$$Base,((int)((int)&autostart$$Limit-(int)&autostart$$Base))/4);//�Զ������߳�
	while(1){
		while(process_run()> 0);//�߳�ѭ��  
		idle_count++;  	
	}
//	return 0;
}


