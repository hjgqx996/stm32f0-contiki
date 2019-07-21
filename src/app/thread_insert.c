#include "includes.h"


/*===================================================
                �������
 �ϸ��� <ͨ������籦�����߼�.pdf> ���̿���								
====================================================*/
#define timeout()  ((time(0)-end)<0x80000000)
#define wait_timeout(condition,to,linenow)  end=time(0)+to;line=linenow+1;case linenow+1: \
														if(((time(0)-end)>0x80000000) && (!(condition)) )return;
																												
/*-----------------------------
1.arg  0xFF (��λ״̬��)
       0xFE (����ǰ״̬)
			 0xFD (�ָ������״̬)
2.���й���
		
	EXTI(�ⲿ�ж�)---->��λ״̬��--->insert�߳�����״̬��(ʶ��,��磬����)
		
-------------------------------*/
static U8 _line[CHANNEL_MAX]={0};//��ǰ״̬
static U8 _last[CHANNEL_MAX]={0};//��һ��״̬
static U16 _end[CHANNEL_MAX]={0}; //��ʱ

#define line _line[ch-1]
#define end _end[ch-1]
#define last _last[ch-1]
void charge_fsm(U8 ch,void*arg)
{

	Channel*pch=channel_data_get(ch);	//�ֵ�����
	if(pch==NULL)return; 
	
	
	//״̬����
	
	
	switch(line)
	{	
		//��ʼ
		case 0:
						last=end=0;
			break;
		//����
		case 1:	if(isvalid_baibi()){line++; last=1;}break; 
		
		//ʶ��/��ʶ��
		case 2:
						wait_timeout(is_readok(), 10000, 2);//10�����Ƿ�ʶ��
						/*---------------��ʶ��---------------------------------*/
						if(is_readok())
						{						
							if(pch->Ufsoc>0){line=20;return;} //����>0===>ֹͣ���
							else {
								line=10;
								request_charge_on(ch,600);      //==>���10����
								return;
							}
							
						/*---------------����ʶ��---------------------------------*/
					 }else{	
						if(last==1){
							line=4;
							request_charge_on(ch,5);			//==>���5��			
						}else if(last==4){
							line=0;                       //���5���޷�ʶ�𣬸�λ
						}
						return;
					 }
		break;

		//���5��
		case 4:
						wait_timeout(isout5v(), 7*3600*1000, 4);//7Сʱ���Ƿ����
						
						if(isout5v())//�����
						{
							wait_timeout(0,5000, 6);//�ȴ�5��
							line=2;last=4;return;   //==����ʶ��һ��
						}
						else{line=0;return;}//�����//��λ
						break;
			 
						
		//���10����
		case 10:
						wait_timeout(isout5v(), 7*3600*1000, 10);//7Сʱ���Ƿ����					
						if(isout5v())//�����
						{
							wait_timeout((pch->Ufsoc>0),600*1000, 12);  //10���ӵ���>0
							line=20;
							return;
						}
						//�����
						else{line=20;return;}     //�����==>ֹͣ���
						break;						

		//ֹͣ���
		case 20:break;
		
		break;
	
	}
	
	
	
	
	
//	Start(){
//			memset(fsm,0,sizeof(FSM));
//			if(isvalid_baibi())goto ʶ��;
//	}
//	State(ʶ��)
//	{
//		wait_timeout(is_readok(),2000,10,{goto���7Сʱ;},{goto ���5��;});//�ȴ���,�������� thread_channel��ʵ��
//	}
//	
//	State(���5��)
//	{
//		request_charge_on(ch,5);
//		wait_timeout(isout5v(),25200000,1000,{break;},{fsm->line=0;return;});//�ȴ�7Сʱֱ�����Գ��,��ʱ��ͷ��ʼ
//		waitms(5000);
//		goto ��һ��ʶ���籦;
//	}
//	State(��һ��ʶ���籦)
//	{
//		wait_timeout(is_readok(),2000,10,
//		{
//			if(pch->Ufsoc==0)goto ���10����;//����Ϊ0���10����
//			if(pch->Ufsoc>0)goto ֹͣ;
//		},{goto ���10����;}); 
//	}
//	State(���10����)
//	{
//		request_charge_on(ch,600);
//		wait_timeout(isout5v(),25200000,1000,{break;},{fsm->line=0;return;});//�ȴ�7Сʱֱ�����Գ��,��ʱ��ͷ��ʼ
//		wait_timeout((pch->Ufsoc>0),600000,1000,{goto ֹͣ;},{goto ֹͣ;});  //���10����ֱ������>0
//	}
//	State(ֹͣ)
//	{
//		
//	}
//	State(���7Сʱ)
//	{
//		
//		//������
//		
////		//�Ѿ����
////		if(queue_isok(ch)==1)
////		{
////			//���5V
////		
////			//�Ѿ����7Сʱ->�������
////			
////			//����<200mA,����120��->�������
////			
////		}else{
////			
////		  //�����ʱ
////			
////			//�����5V
////		}
//		
//	}
//	
//	State(�������)
//	{
//	  //����>95%:������
//		//����>=85%,<=95%:1Сʱ����1�Σ�3�λ���
//		//����<85%,3Сʱ����һ�Σ����޻���
//	
//	}
//  Default()
	
}

/*===================================================
                ȫ�ֺ���
====================================================*/
AUTOSTART_THREAD_WITH_TIMEOUT(insert)
{
	static int i= 0;
	PROCESS_BEGIN();
	while(1)
	{
		for(i=1;i<=CHANNEL_MAX;i++)
		{
			charge_fsm(i,NULL);
		}
		os_delay(insert,50);
		ld_iwdg_reload();
	}
	PROCESS_END();
}

