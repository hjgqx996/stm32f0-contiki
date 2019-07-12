#include "iic.h"
#include "string.h"


/*===================================================
                配置文件
====================================================*/
#define IR_DATA_MAX     13   //接收最大数据
#define IR_CHANNEL_MAX  5    //最大仓道数
#define FSM_TICK        100  //最小中断时间100us

#define ir_lock()
#define ir_unlock()

/*===================================================
                类型
====================================================*/
/*红外收发状态*/
typedef enum{	
	IR_Error_Header=-1,     //读取前导码出错
	IR_Error_Data=-2,       //读取数据出错
	IR_State_NULL =0,
	IR_State_OK   =2,       //读取数据正确
}IR_STATE;

typedef struct{
	U8 io_ir;        //红外发送io
	U8 io_re;        //红外接收io
	U8 cmd;          //发送命令
	U8 data[16];     //接收数据
	U8 wanlen;       //要接收的数据长度
	U8 len;          //实际接收到的数据长度
	BOOL start;      //TRUE: 开始  FALSE:结束
	int state;  //错误码
	
	int counter;     //计数
	U8 tmp;          //缓存一字节
	
	////////////////////////////////
	FSM fsm;         //状态机私有变量
}IR_Type;

static IR_Type irs[IR_CHANNEL_MAX];

/*===================================================
                本地函数
====================================================*/
/*使用定时器3,频率100us*/
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
  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
  TIM_Cmd(TIM3, ENABLE);
}

/*红外读状态机*/
static void ir_fsm(IR_Type*pir)
{
	FSM*fsm=&pir->fsm;
	
	Start(开始)
	{
    if(pir->start==TRUE)
			goto 发送命令码;
	}
	
	
	State(发送命令码)
	{
		 ld_gpio_set(pir->io_ir,1);
	   WaitMs(100);
		 ld_gpio_set(pir->io_ir,0);
	   WaitMs(100);
		
		//发送指令
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
		goto 读取前导码;
	}
	
	
	
	State(读取前导码)
	{
		//等待拉高，超时60ms
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
		
	  //9ms高电平,7ms-10ms为正常
		pir->counter=0;
		while(ld_gpio_get(pir->io_re)==1)
		{
			WaitUs(200);
			pir->counter+=200;
			if(pir->counter>10000)
				goto Header_Error;
		}
		if(pir->counter<7000)goto Header_Error;
		
		//4.5ms低电平,2.5ms-5ms为正常时间
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
		
		goto 读取数据;
	}
	
	State(读取数据)
	{
		for(fsm->i=0;fsm->i<pir->wanlen;fsm->i++)
		{
			pir->tmp=0;
			for(fsm->j=0;fsm->j<8;fsm->j++)
			{
			
				//判断高电平时间 200-600
				pir->counter=0;
				while(ld_gpio_get(pir->io_re)==1)
				{
					WaitUs(100);
					pir->counter+=100;
					if(pir->counter>600)goto Data_Error;
				}
				if(pir->counter<200)goto Data_Error;
				
				//读取低电平时间，超时1.7ms
				pir->counter=0;
				while(ld_gpio_get(pir->io_re)==0)
				{
					WaitUs(200);
					pir->counter+=200;
					if(pir->counter>1700)goto Data_Error;
				}
				
				//200us-600us :低电平    1200us-1700us:高电平
				if(pir->counter <200)goto Data_Error;
				pir->tmp>>=1;
				if(pir->counter>=1200 && pir->counter<=1700){
					pir->tmp|=0x80;
				}			
			}
			
			//读入一个字节后，读取停止码 H=200us-600us  L=700us-1100us
			pir->counter=0;
			while(ld_gpio_get(pir->io_re)==1)
			{
				WaitUs(100);
				pir->counter+=100;
				if(pir->counter>600)goto Data_Error;
			}	
			if(pir->counter<200)goto Data_Error;
				
			//最后一个字节，不读取低电平
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
			//保存一个字节
			pir->data[fsm->i]=pir->tmp;
		}
		
		//接收成功
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
                全局函数
====================================================*/
/*初始化配置
* ch    :仓道号 1-n
* io_ir :发送端口
* io_re :接收端口
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

//定时器中断服务，用于收发时序
void ld_ir_timer_100us(void)
{
	int i=0;
	for(;i<IR_CHANNEL_MAX;i++)
		ir_fsm(&irs[i]);
}

//开始读取红外数据
BOOL ld_ir_read_start(U8 ch,U8 cmd,U8 wanlen)
{
	ch-=1;
	ir_lock();
	if(ch < IR_CHANNEL_MAX && irs[ch].start==FALSE)
  {
		//开始读
		irs[ch].cmd = cmd;
		irs[ch].wanlen=wanlen;
		irs[ch].start=TRUE;
		
		//复位状态机
		irs[ch].fsm.state=0;
		irs[ch].fsm.name=NULL;
		
		ir_unlock();
		return TRUE;
	}
	ir_unlock();
	return FALSE;
}

/*查看是否读完成
* return : <0：error
*        :  0: 无操作
*        :  1: 正在读
*        :  2: 读正确
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
                测试红外读
====================================================*/
#include "contiki.h"
static struct etimer et_testir;
PROCESS(testir_thread, "归还任务");
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