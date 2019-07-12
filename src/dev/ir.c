#include "iic.h"
#include "string.h"


/*===================================================
                �����ļ�
====================================================*/
#define IR_DATA_MAX     13   //�����������
#define IR_CHANNEL_MAX  5    //���ֵ���
#define FSM_TICK        100  //��С�ж�ʱ��100us

#define ir_lock()
#define ir_unlock()

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
	U8 io_ir;        //���ⷢ��io
	U8 io_re;        //�������io
	U8 cmd;          //��������
	U8 data[16];     //��������
	U8 wanlen;       //Ҫ���յ����ݳ���
	U8 len;          //ʵ�ʽ��յ������ݳ���
	BOOL start;      //TRUE: ��ʼ  FALSE:����
	int state;  //������
	
	int counter;     //����
	U8 tmp;          //����һ�ֽ�
	
	////////////////////////////////
	FSM fsm;         //״̬��˽�б���
}IR_Type;

static IR_Type irs[IR_CHANNEL_MAX];

/*===================================================
                ���غ���
====================================================*/
/*ʹ�ö�ʱ��3,Ƶ��100us*/
#include "stm32f0xx_tim.h"
static void timer_init()
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
  /* Enable the TIM3 gloabal Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 50-1;
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t) (SystemCoreClock  / 500000) - 1;  //timer3 counter =500KHz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* TIM3 enable counter */
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
  TIM_Cmd(TIM3, ENABLE);
}

/*�����״̬��*/
static void ir_fsm(IR_Type*pir)
{
	FSM*fsm=&pir->fsm;
	
	Start(��ʼ)
	{
    if(pir->start==TRUE)
			goto ����������;
	}
	
	
	State(����������)
	{
		 ld_gpio_set(pir->io_ir,1);
	   WaitMs(100);
		 ld_gpio_set(pir->io_ir,0);
	   WaitMs(100);
		
		//����ָ��
		for(fsm->i=0;fsm->i<(pir->cmd-1);fsm->i++)
		{
			ld_gpio_set(pir->io_ir,1);	//H
			WaitMs(2);
			ld_gpio_set(pir->io_ir,0);  //L
			WaitMs(2);
		}
		ld_gpio_set(pir->io_ir,1);    //H
		WaitMs(3);
		ld_gpio_set(pir->io_ir,0);    //L
		WaitMs(50);
		goto ��ȡǰ����;
	}
	
	
	
	State(��ȡǰ����)
	{
		//�ȴ����ߣ���ʱ60ms
		pir->counter=0;
		while(ld_gpio_get(pir->io_re)!=1)
		{
			WaitUs(100);
			pir->counter+=100;
			if(pir->counter>60000)
			{
				goto Header_Error;
			}
		}
		
	  //9ms�ߵ�ƽ,7ms-10msΪ����
		pir->counter=0;
		while(ld_gpio_get(pir->io_re)==1)
		{
			WaitUs(200);
			pir->counter+=200;
			if(pir->counter>10000)
				goto Header_Error;
		}
		if(pir->counter<7000)goto Header_Error;
		
		//4.5ms�͵�ƽ,2.5ms-5msΪ����ʱ��
		pir->counter=0;
		while(ld_gpio_get(pir->io_re)!=1)
		{
			WaitUs(200);
			pir->counter+=200;
			if(pir->counter>5000)
			{
				goto Header_Error;
			}
		}
		if(pir->counter<2500)goto Header_Error;
		
		goto ��ȡ����;
	}
	
	State(��ȡ����)
	{
		for(fsm->i=0;fsm->i<pir->wanlen;fsm->i++)
		{
			pir->tmp=0;
			for(fsm->j=0;fsm->j<8;fsm->j++)
			{
			
				//�жϸߵ�ƽʱ�� 200-600
				pir->counter=0;
				while(ld_gpio_get(pir->io_re)==1)
				{
					WaitUs(100);
					pir->counter+=100;
					if(pir->counter>600)goto Data_Error;
				}
				if(pir->counter<200)goto Data_Error;
				
				//��ȡ�͵�ƽʱ�䣬��ʱ1.7ms
				pir->counter=0;
				while(ld_gpio_get(pir->io_re)==0)
				{
					WaitUs(200);
					pir->counter+=200;
					if(pir->counter>1700)goto Data_Error;
				}
				
				//200us-600us :�͵�ƽ    1200us-1700us:�ߵ�ƽ
				if(pir->counter <200)goto Data_Error;
				pir->tmp>>=1;
				if(pir->counter>=1200 && pir->counter<=1700){
					pir->tmp|=0x80;
				}			
			}
			
			//����һ���ֽں󣬶�ȡֹͣ�� H=200us-600us  L=700us-1100us
			pir->counter=0;
			while(ld_gpio_get(pir->io_re)==1)
			{
				WaitUs(100);
				pir->counter+=100;
				if(pir->counter>600)goto Data_Error;
			}	
			if(pir->counter<200)goto Data_Error;
				
			//���һ���ֽڣ�����ȡ�͵�ƽ
			if(fsm->i!=(pir->wanlen-1))
			{
				pir->counter=0;
				while(ld_gpio_get(pir->io_re)==0)
				{
					WaitUs(100);
					pir->counter+=100;
					if(pir->counter>1100)goto Data_Error;
				}	
				if(pir->counter<700)goto Data_Error;			
			}
			//����һ���ֽ�
			pir->data[fsm->i]=pir->tmp;
		}
		
		//���ճɹ�
		pir->counter = 0;
		pir->start=FALSE;
		pir->state=IR_State_OK;
		pir->fsm.state=0;
	}
	default:{}}
		
	return ;
		
	Header_Error:
	pir->counter = 0;
	pir->start=FALSE;
	pir->state=IR_Error_Header;
	pir->fsm.state=0;
	return;
		
		
	Data_Error:
	pir->counter = 0;
	pir->start=FALSE;
	pir->state=IR_Error_Data;
	pir->fsm.state=0;		
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
	int i=0;
	for(;i<IR_CHANNEL_MAX;i++)
		ir_fsm(&irs[i]);
}

//��ʼ��ȡ��������
BOOL ld_ir_read_start(U8 ch,U8 cmd,U8 wanlen)
{
	ch-=1;
	ir_lock();
	if(ch < IR_CHANNEL_MAX && irs[ch].start==FALSE)
  {
		//��ʼ��
		irs[ch].cmd = cmd;
		irs[ch].wanlen=wanlen;
		irs[ch].start=TRUE;
		
		//��λ״̬��
		irs[ch].fsm.state=0;
		irs[ch].fsm.name=NULL;
		
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
	ld_ir_init(1,17,22);
	ld_ir_init(2,18,23);
	ld_ir_init(3,19,24);
	ld_ir_init(4,20,25);
	ld_ir_init(5,21,26);
	id_ir_timer_init();
	while(1)
	{
		
		ld_ir_read_start(2,0x10,7);
		
    os_delay(et_testir,2000);
	}

	PROCESS_END();
}
AUTOSTART_PROCESSES(testir_thread);