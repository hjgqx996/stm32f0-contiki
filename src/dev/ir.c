#include "types.h"
#include "string.h"
#include "lib.h"
#include "channel.h"
#include "dev.h"
#include "config.h"

#if  NOT_USING_IR == 0

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
extern void ld_gpio_set(U32 index,U8 value);
#define ir(data)       ld_gpio_set(io_ir,data)

//��ȡ�����ƽ
#define re()           ld_gpio_get(io_re)

//�ȴ�reֱ��re!=level,�ȴ�ʱ��ƬΪtick_us,�ȴ��ܳ�ʱΪtimeout_us
//Ĭ�ϵȴ�ʱ��ƬΪFSM_TICK,�����ж���
#define FSM_WAIT_TICK  FSM_TICK
#define wait_re_until_not(level,timeout_us) \
    /*��ʱ��0*/   irs.counter=0; \
    /*�����ƽ*/  while(re()==level){ \
		/*�ȴ�ʱ��Ƭ*/	  waitus(FSM_WAIT_TICK); irs.counter+=FSM_WAIT_TICK; \
		/*�жϳ�ʱ�˳�*/  if(irs.counter>timeout_us)break; \
		              }
		
//�ж�һ����ƽ����һ��ʱ�䷶Χ(min_us,max_us)
#define if_re_not_between(min_us,max_us)     if((irs.counter<min_us)|| (irs.counter>max_us) )
#define if_re_between(min_us,max_us)         if((irs.counter>=min_us) && (irs.counter<=max_us) )
#define if_re_higher(max_us)                 if(irs.counter>max_us)
#define if_re_lower(min_us)                  if(irs.counter<min_us)

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
	IR_STATE state;        			//������		
	S32 counter;     			//����
	U8 data[IR_DATA_MAX]; //��������
	U8 tmp;          			//����һ�ֽ�
	
	BOOL inited;          //��ʼ����־��δ��ʼ����״̬����������
	////////////////////////////////
	FSM fsm;         //״̬��˽�б���
}IR_Type;

U8 prebuffer[16];
static volatile IR_Type irs;
U8 afterbuffer[16];
/*===================================================
                ���غ���
====================================================*/
/*����״̬��(α����)
* ����������--->��ȡǰ����--->��������--->�ɹ�
* 1.״̬����ʼ��ʹ�� ld_ir_read_start
* 2.��ȡ�Ƿ�ɹ�ʹ�� ld_ir_read_isok
*/
static U32 fsm_time = 0;
void ld_ir_timer_100us(void)
{
	FSM*fsm=(FSM*)&irs.fsm;
  U8 io_re=irs.io_re;
	U8 io_ir=irs.io_ir;
	ld_gpio_refresh();
	if( (irs.start==FALSE) || (irs.inited==FALSE) )return;	
	fsm_time+= FSM_TICK;
	//////////////////////////////////
	Start()
	{
		/*---------- �ߵ�100ms --------------------- */
		ir(HIGH);
		waitms(100);
		ir(LOW);
		waitms(100);
		
		/*---------- ����ָ�� --------------------- */
		for(fsm->i=0;fsm->i<(irs.cmd-1);fsm->i++)
		{
			ir(HIGH);
			waitus(2000);//waitms(2);
			ir(LOW);
			waitus(2000);//waitms(2);
		}
		
		ir(HIGH);     
		waitms(3);
		ir(LOW); 
		waitms(50);
		goto READ_TYPE_CMD;
	}
	
	State(READ_TYPE_CMD)
	{
		fsm_time=0;
		/*---------- ��ȡ���� --------------------- */
		wait_re_until_not(LOW,60000)  
		if_re_higher(60000)
			goto Header_Error;
		
	  //9ms�ߵ�ƽ,7ms-11msΪ����,��ʱ11ms//ʵ��9300-9500
		wait_re_until_not(HIGH,14000)  
		if_re_not_between(7000,14000)
			goto Header_Error;


		//4.5ms�͵�ƽ,2.5ms-5msΪ����ʱ��,��ʱ5ms//ʵ��4600
		wait_re_until_not(LOW,7000)  
		if_re_not_between(2000,7000)
			goto Header_Error;

		/*---------- ��ȡ���� --------------------- */

		for(fsm->i=0;fsm->i<irs.wanlen;fsm->i++)
		{
			irs.tmp=0;
			for(fsm->j=0;fsm->j<8;fsm->j++)
			{
				//�ߵ�ƽ200-600,��ʱ600//ʵ��300-400-500us
				wait_re_until_not(HIGH,800)  
				if_re_not_between(200,800)
					goto Data_Error;

				//�͵�ƽ200-1700����ʱ2ms
				wait_re_until_not(LOW,2400) 
				if_re_not_between(200,2400)
					goto Data_Error;
				
			  irs.tmp>>=1;
				
				//200us-600us :�͵�ƽ    //ʵ��300-400
				if_re_lower(200)
					goto Data_Error;
			
				//1200us-1700us:�ߵ�ƽ  //ʵ��1500-1600
				if_re_between(1000,2400)
					irs.tmp|=0x80;//����һλ����		
			}
			
			//��ȡֹͣ�� H=200us-600us  //ʵ��300-400-500
			wait_re_until_not(HIGH,800)  
			if_re_not_between(200,800)
				goto Data_Error;
			
			//��ȡֹͣ�� L=700us-1400us //ʵ�� 1000-1100-1200
			if(fsm->i!=(irs.wanlen-1))//��һ���ֽڲ���ȡ
			{
				wait_re_until_not(LOW,1800)  		
        if_re_not_between(800,1800)
					goto Data_Error;				
			}
			//����һ���ֽ�
			irs.data[fsm->i]=irs.tmp;
		}
		
		//���ճɹ�
		irs.counter = 0;
		irs.start=FALSE;
		irs.state=IR_State_OK;
		irs.fsm.line=0;
		irs.fsm.save=0;
		irs.fsm.end=0;
	}
	Default()
	ld_gpio_refresh();
	return ;
		
	Header_Error:
	#ifdef USING_DEBUG_INFO
		 ld_debug_printf(1,irs.io_ir|0x80,irs.cmd ,2);//headr ������ type=1
	#endif
	irs.counter = 0;
	irs.start=FALSE;
	irs.state=IR_Error_Header;
	irs.fsm.line=0;
	irs.fsm.save=0;
	irs.fsm.end=0;
	return;
		
		
	Data_Error:
	#ifdef USING_DEBUG_INFO
		 ld_debug_printf(2,irs.io_ir|0x80,irs.cmd,2);//data ������ type=2
	#endif
	irs.counter = 0;
	irs.start=FALSE;
	irs.state=IR_Error_Data;
	irs.fsm.line=0;
	irs.fsm.save=0;
	irs.fsm.end=0;	
}

/*===================================================
                ȫ�ֺ���
====================================================*/
//��ʼ��ȡ��������   (ch:1-n,opposite:TRUE����(δʹ��), cmd ����, ����)
BOOL ld_ir_read_start(U8 ir,U8 re,U8 cmd,U8 wanlen)
{
	irs.start=FALSE;
	ir_lock();
	//��ʼ��
	memset((void*)&irs,0,sizeof(irs));
	irs.io_ir = ir;
	irs.io_re = re;
	irs.cmd = cmd;
	irs.wanlen=wanlen;
	fsm_time = 0;
	irs.inited=TRUE;
	irs.start=TRUE;
	ir_unlock();
	return TRUE;
}

/*�鿴�Ƿ�����
* return : <0��error
*        :  0: �޲���
*        :  1: ���ڶ�
*        :  2: ����ȷ
*/
int ld_ir_read_isok(U8*dataout,U8 size)
{
	int err = -1;
  ir_lock();
	if(irs.inited==FALSE)goto END;
	if(irs.start==FALSE)
	{
		err=(int)irs.state;
	}else{
		err = 1;
	}
	//��ȷʱ����������:��ʽ�����
	if(err==2 && dataout!= NULL)
	{
		//��ʽ�������������
		switch(irs.cmd)
		{
			case RC_READ_ID:
			{
				U8 cs = 0xFF-cs8(irs.data,6);
				U8 i = 0;
				if(cs!=irs.data[6]){
					err=-1;
					goto END;
				}//����ʧ��
				memset(dataout,0,10);
				for(i=0;i<6;i++){dataout[9-i]=irs.data[i];}
			}break;
			
			case RC_READ_DATA://[0] �汾�� [1] ���� [2] �¶� [3] ������ [4-5] ѭ������ [6-7] ���� [8-9] ��о��ѹ [10-11] ���� (��λ��ǰ)
			{
				U8 cs = 0xFF-cs8(irs.data,12);
				if(cs!=irs.data[12]){
					err=-1;
					goto END;
				}//����ʧ��
				dataout[0]=irs.data[0];
				dataout[1]=irs.data[3];
				dataout[2]=irs.data[4];
				dataout[3]=irs.data[11];
				dataout[4]=irs.data[6];
				dataout[5]=irs.data[5];
				dataout[6]=irs.data[8];
				dataout[7]=irs.data[7];
				dataout[8]=irs.data[10];
				dataout[9]=irs.data[9];
				dataout[10]=irs.data[2];
				dataout[11]=irs.data[1];
			}break;
			
			case RC_LOCK:         //[0]�����־
			case RC_UNLOCK:
			case RC_UNLOCK_1HOUR:
			{
				if((0xFF-irs.data[0])!=irs.data[1])
				{
					err=-1;
					goto END;
				}//����ʧ��
				dataout[0] = irs.data[0];
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
//	  ld_ir_read_start(3,FALSE,RC_READ_ID,7);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(3,dataout,10);
//		
//		ld_ir_read_start(3,FALSE,RC_READ_DATA,13);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(3,dataout,13);
//		
//		ld_ir_read_start(3,FALSE,RC_LOCK,2);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(3,dataout,1);
//		
//		ld_ir_read_start(3,FALSE,RC_UNLOCK,2);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(3,dataout,1);

//		ld_ir_read_start(3,FALSE,RC_UNLOCK_1HOUR,2);
//		os_delay(ir_test,1000);
//		ld_ir_read_isok(3,dataout,1);
//		
//		os_delay(ir_test,1000);
//	}
//	PROCESS_END();
//}

#endif


