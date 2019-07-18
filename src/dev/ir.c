#include "types.h"
#include "string.h"
#include "lib.h"
#include "channel.h"
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
	
	BOOL inited;          //��ʼ����־��δ��ʼ����״̬����������
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
static void ir_fsm(IR_Type*pir,FSM*fsm,U32 tick)
{
	fsm_time_add(tick);//״̬��ʱ��	
	//////////////////////////////////
	Start(��ʼ)
	{
    if( (pir->start==TRUE) && (pir->inited==TRUE) )
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
#include "channel.h"
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
	irs[ch].inited = TRUE;
	ir_unlock();
}
void ld_ir_timer_init(void)
{
	timer_init();
}

//��ʱ���жϷ��������շ�ʱ��
void ld_ir_timer_100us(void)
{
	int i=0;
	for(;i<IR_CHANNEL_MAX;i++)
		ir_fsm(&irs[i],&irs[i].fsm,((i==0)?FSM_TICK:0));
}

//��ʼ��ȡ��������   (ch:1-n,opposite:TRUE����(δʹ��), cmd ����, ����)
BOOL ld_ir_read_start(U8 ch,BOOL opposite,U8 cmd,U8 wanlen)
{
	if(ch>IR_CHANNEL_MAX)return FALSE;
	ch-=1;
	ir_lock();
	if(irs[ch].inited==FALSE){ir_unlock();return FALSE;}//δ��ʼ��
	
	if(irs[ch].start==TRUE){ir_unlock();return TRUE;}//�����Ѿ���ʼ��
	
	if(irs[ch].start==FALSE)//����δ��ʼ��
  {
		//��ʼ��
		irs[ch].cmd = cmd;
		irs[ch].wanlen=wanlen;
		irs[ch].start=TRUE;
		memset(irs[ch].data,0,IR_DATA_MAX);
    memset(&irs[ch].fsm,0,sizeof(FSM));//��λ״̬��
		ir_unlock();
		return TRUE;
	}
	ir_unlock();
	return FALSE;
}

BOOL ld_ir_busy(U8 ch)
{
	BOOL r = 0;
	ch-=1;
	ir_lock();
	if(ch>=IR_CHANNEL_MAX){ir_unlock();return FALSE;}
	r = irs[ch].start;
	ir_unlock();
	return r;
}


/*�鿴�Ƿ�����
* return : <0��error
*        :  0: �޲���
*        :  1: ���ڶ�
*        :  2: ����ȷ
*/
int ld_ir_read_isok(U8 ch,U8*dataout,U8 size)
{
	int err = -1;
	ch-=1;
  ir_lock();
	if(ch>=IR_CHANNEL_MAX||irs[ch].inited==FALSE)goto END;
	if(irs[ch].start==FALSE)
	{
		err=(int)irs[ch].state;
	}else{
		err = 1;
	}
	//��ȷʱ����������:��ʽ�����
	if(err==2 && dataout!= NULL)
	{
		//��ʽ�������������
		switch(irs[ch].cmd)
		{
			case RC_READ_ID:
			{
				U8 cs = 0xFF-cs8(irs[ch].data,6);
				U8 i = 0;
				if(cs!=irs[ch].data[6]){err=-1;goto END;}//����ʧ��
				memset(dataout,10,0);
				for(i=0;i<6;i++){dataout[9-i]=irs[ch].data[i];}
			}break;
			
			case RC_READ_DATA://[0] �汾�� [1] ���� [2] �¶� [3] ������ [4-5] ѭ������ [6-7] ���� [8-9] ��о��ѹ [10-11] ���� (��λ��ǰ)
			{
				U8 cs = 0xFF-cs8(irs[ch].data,12);
				if(cs!=irs[ch].data[12]){err=-1;goto END;}//����ʧ��
				dataout[0]=irs[ch].data[0];
				dataout[1]=irs[ch].data[3];
				dataout[2]=irs[ch].data[4];
				dataout[3]=irs[ch].data[11];
				dataout[4]=irs[ch].data[6];
				dataout[5]=irs[ch].data[5];
				dataout[6]=irs[ch].data[8];
				dataout[7]=irs[ch].data[7];
				dataout[8]=irs[ch].data[10];
				dataout[9]=irs[ch].data[9];
				dataout[10]=irs[ch].data[2];
				dataout[11]=irs[ch].data[1];
			}break;
			
			case RC_LOCK:         //[0]�����־
			case RC_UNLOCK:
			case RC_UNLOCK_1HOUR:
			{
				if((0xFF-irs[ch].data[0])!=irs[ch].data[1]){err=-1;goto END;}//����ʧ��
				dataout[0] = irs[ch].data[0];
			}break;
			case RC_OUTPUT:
				break;
      default:goto END;
		}

	}
	END:
	ir_unlock();
	return err;
}

///*===================================================
//                ���Ժ����
//====================================================*/
//#include "contiki.h"
//AUTOSTART_THREAD_WITH_TIMEOUT(ir_test)
//{
//	U8 dataout[13];
//	PROCESS_BEGIN();
//  os_delay(ir_test,500);
//	while(1)
//	{
//	  ld_ir_read_start(2,FALSE,RC_READ_ID,7);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(2,dataout,10);
		
//		ld_ir_read_start(2,FALSE,RC_READ_DATA,13);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(2,dataout,13);
//		
//		ld_ir_read_start(2,FALSE,RC_LOCK,2);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(2,dataout,1);
//		
//		ld_ir_read_start(2,FALSE,RC_UNLOCK,2);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(2,dataout,1);

//		ld_ir_read_start(2,FALSE,RC_UNLOCK_1HOUR,2);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(2,dataout,1);
		
//		os_delay(ir_test,1000);
//	}
//	PROCESS_END();
//}

