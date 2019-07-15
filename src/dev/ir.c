#include "types.h"
#include "string.h"


/*===================================================
                �����ļ�
====================================================*/
#define IR_DATA_MAX     13   //�����������
#define IR_CHANNEL_MAX  5    //���ֵ���
#define FSM_TICK        100  //��С�ж�ʱ��100us

//����(ֻ����ռʽ�ں���ʹ��)
#define ir_lock()
#define ir_unlock()

//������͵�ƽ
#define ir(data)       ld_gpio_set(pir->io_ir,data)

//��ȡ�����ƽ
#define re()           ld_gpio_get(pir->io_re)

//�ȴ�reֱ��re!=level,�ȴ�ʱ��ƬΪtick_us,�ȴ��ܳ�ʱΪtimeout_us
//Ĭ�ϵȴ�ʱ��ƬΪFSM_TICK,�����ж���
#define FSM_WAIT_TICK  FSM_TICK
#define wait_re_until_not(level,timeout_us) \
    /*��ʱ��0*/   pir->counter=0; \
    /*�����ƽ*/  while(re()==level){ \
		/*�ȴ�ʱ��Ƭ*/	  waitus(FSM_WAIT_TICK); pir->counter+=FSM_WAIT_TICK; \
		/*�жϳ�ʱ�˳�*/  if(pir->counter>timeout_us)break; \
		              }
		
//�ж�һ����ƽ����һ��ʱ�䷶Χ(min_us,max_us)
#define if_re_not_between(min_us,max_us)     if(pir->counter<min_us||pir->counter>max_us)
#define if_re_between(min_us,max_us)         if(pir->counter>=min_us&&pir->counter<=max_us)
#define if_re_higher(max_us)                 if(pir->counter>max_us)
#define if_re_lower(min_us)                  if(pir->counter<min_us)

/*===================================================
                ����
====================================================*/
/*�����շ�״̬*/
typedef enum{	
	IR_Error_Header=-1,     //��ȡǰ�������
	IR_Error_Data=-2,       //��ȡ���ݳ���
	IR_State_NULL =0,
	IR_State_OK   =2,       //��ȡ������ȷ
}IR_STATE;

typedef struct{
	U8 io_ir;        			//���ⷢ��io
	U8 io_re;        			//�������io
	U8 cmd;          			//��������	
	U8 wanlen;       			//Ҫ���յ����ݳ���
	U8 len;          			//ʵ�ʽ��յ������ݳ���
	BOOL start;      			//TRUE: ��ʼ  FALSE:����
	S8 state;        			//������		
	S32 counter;     			//����
	U8 data[IR_DATA_MAX]; //��������
	U8 tmp;          			//����һ�ֽ�
	
	////////////////////////////////
	FSM fsm;         //״̬��˽�б���
}IR_Type;

static IR_Type irs[IR_CHANNEL_MAX];

/*===================================================
                ���غ���
====================================================*/
/*����״̬��(α����)
* ����������--->��ȡǰ����--->��������--->�ɹ�
* 1.״̬����ʼ��ʹ�� ld_ir_read_start
* 2.��ȡ�Ƿ�ɹ�ʹ�� ld_ir_read_isok
*/
static void ir_fsm(IR_Type*pir,FSM*fsm)
{
	fsm_time_add(FSM_TICK);//״̬��ʱ��	
	//////////////////////////////////
	Start(��ʼ)
	{
    if(pir->start==TRUE)
			goto ����������;
	}
	//////////////////////////////////
	State(����������)
	{
		//�ߵ�100ms
		ir(HIGH);
		waitms(100);
		ir(LOW);
		waitms(100);
		
		//����ָ��
		for(fsm->i=0;fsm->i<(pir->cmd-1);fsm->i++)
		{
			ir(HIGH);
			waitms(2);
			ir(LOW); 
			waitms(2);
		}
		ir(HIGH);     
		waitms(3);
		ir(LOW); 
		waitms(50);
		goto ��ȡǰ����;
	}
	//////////////////////////////////
	State(��ȡǰ����)
	{
		//�ȴ�re����,��ʱʱ��60ms   //ʵ��10300
		wait_re_until_not(LOW,60000)  
		if_re_higher(60000)
			goto Header_Error;
		
	  //9ms�ߵ�ƽ,7ms-11msΪ����,��ʱ11ms//ʵ��9300
		wait_re_until_not(HIGH,11000)  
		if_re_not_between(7000,11000)
			goto Header_Error;


		//4.5ms�͵�ƽ,2.5ms-5msΪ����ʱ��,��ʱ5ms//ʵ��4600
		wait_re_until_not(LOW,5000)  
		if_re_not_between(2500,5000)
			goto Header_Error;

		goto ��ȡ����;
	}
	//////////////////////////////////
	State(��ȡ����)
	{
		for(fsm->i=0;fsm->i<pir->wanlen;fsm->i++)
		{
			pir->tmp=0;
			for(fsm->j=0;fsm->j<8;fsm->j++)
			{
				//�ߵ�ƽ200-600,��ʱ600//ʵ��300
				wait_re_until_not(HIGH,600)  
				if_re_not_between(200,600)
					goto Data_Error;

				//�͵�ƽ200-1700����ʱ2ms
				wait_re_until_not(LOW,2000) 
				if_re_not_between(200,2000)
					goto Data_Error;
				
			  pir->tmp>>=1;
				
				//200us-600us :�͵�ƽ    //ʵ��300-400
				if_re_lower(200)
					goto Data_Error;
			
				//1200us-1700us:�ߵ�ƽ  //ʵ��1600
				if_re_between(1100,2000)
					pir->tmp|=0x80;//����һλ����		
			}
			
			//��ȡֹͣ�� H=200us-600us  //ʵ��300-400
			wait_re_until_not(HIGH,600)  
			if_re_not_between(200,600)
				goto Data_Error;
			
			//��ȡֹͣ�� L=700us-1400us //ʵ�� 1100-1200
			if(fsm->i!=(pir->wanlen-1))//��һ���ֽڲ���ȡ
			{
				wait_re_until_not(LOW,1400)  		
        if_re_not_between(700,1400)
					goto Data_Error;				
			}
			//����һ���ֽ�
			pir->data[fsm->i]=pir->tmp;
		}
		
		//���ճɹ�
		pir->counter = 0;
		pir->start=FALSE;
		pir->state=IR_State_OK;
		pir->fsm.line=0;
		pir->fsm.save=0;
		pir->fsm.end=0;
	}
	Default()
		
	return ;
		
	Header_Error:
	pir->counter = 0;
	pir->start=FALSE;
	pir->state=IR_Error_Header;
	pir->fsm.line=0;
	pir->fsm.save=0;
	pir->fsm.end=0;
	return;
		
		
	Data_Error:
	pir->counter = 0;
	pir->start=FALSE;
	pir->state=IR_Error_Data;
	pir->fsm.line=0;
	pir->fsm.save=0;
	pir->fsm.end=0;	
}

/*===================================================
                ȫ�ֺ���
====================================================*/
/*��ʼ������
* ch    :�ֵ��� 1-n
* io_ir :���Ͷ˿�
* io_re :���ն˿�
*/
void ld_ir_init(U8 ch,U8 io_ir,U8 io_re)
{
	ch-=1;
	ir_lock();
	if(ch>=IR_CHANNEL_MAX){
		ir_unlock();return;
	}
  memset(&irs[ch],0,sizeof(IR_Type));
	irs[ch].io_ir=io_ir;
	irs[ch].io_re=io_re;
	irs[ch].start=FALSE;
	irs[ch].state=IR_State_NULL;
	ir_unlock();
}
void id_ir_timer_init(void)
{
	timer_init();
}

//��ʱ���жϷ��������շ�ʱ��
void ld_ir_timer_100us(void)
{
//	int i=0;
//	for(;i<IR_CHANNEL_MAX;i++)
//		ir_fsm(&irs[i],&irs[i].fsm);
	ir_fsm(&irs[1],&irs[1].fsm);
}

//��ʼ��ȡ��������
BOOL ld_ir_read_start(U8 ch,U8 cmd,U8 wanlen)
{
	if(ch>IR_CHANNEL_MAX)return FALSE;
	ch-=1;
	ir_lock();
	//�����Ѿ���ʼ��
	if(irs[ch].start==TRUE)return TRUE;
	//����δ��ʼ��
	if(irs[ch].start==FALSE)
  {
		//��ʼ��
		irs[ch].cmd = cmd;
		irs[ch].wanlen=wanlen;
		irs[ch].start=TRUE;
		
		//��λ״̬��
    memset(&irs[ch].fsm,0,sizeof(FSM));
	
		ir_unlock();
		return TRUE;
	}
	ir_unlock();
	return FALSE;
}

/*�鿴�Ƿ�����
* return : <0��error
*        :  0: �޲���
*        :  1: ���ڶ�
*        :  2: ����ȷ
*/
int ld_ir_read_isok(U8 ch)
{
	int err = -1;
	ch-=1;
  ir_lock();
	if(ch>=IR_CHANNEL_MAX)goto END;
	if(irs[ch].start==FALSE)
	{
		err=(int)irs[ch].state;
	}else{
		err = 1;
	}
	END:
	ir_unlock();
	return err;
}



/*===================================================
                ���Ժ����
====================================================*/
#include "contiki.h"
static struct etimer et_testir;
PROCESS(testir_thread, "�黹����");
PROCESS_THREAD(testir_thread, ev, data)  
{
	PROCESS_BEGIN();
	ld_ir_init(1,6,7);
	ld_ir_init(2,16,17);
	ld_ir_init(3,26,27);
	ld_ir_init(4,36,37);
	ld_ir_init(5,46,47);
	id_ir_timer_init();
	
	while(1)
	{
//		ld_ir_timer_100us();
//		ld_ir_read_start(1,0x10,7);
		ld_ir_read_start(2,10,7);
//		ld_ir_read_start(3,0x10,7);
//		ld_ir_read_start(4,0x10,7);
//		ld_ir_read_start(5,0x10,7);
		os_delay(et_testir,2000);
	}
	PROCESS_END();
}
AUTOSTART_PROCESSES(testir_thread);
