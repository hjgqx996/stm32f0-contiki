#include "types.h"
#include "string.h"


/*===================================================
                配置文件
====================================================*/
#define IR_DATA_MAX     13   //接收最大数据
#define IR_CHANNEL_MAX  5    //最大仓道数
#define FSM_TICK        100  //最小中断时间100us

//互斥(只在抢占式内核中使用)
#define ir_lock()
#define ir_unlock()

//输出发送电平
#define ir(data)       ld_gpio_set(pir->io_ir,data)

//读取输入电平
#define re()           ld_gpio_get(pir->io_re)

//等待re直到re!=level,等待时间片为tick_us,等待总超时为timeout_us
//默认等待时间片为FSM_TICK,可自行定义
#define FSM_WAIT_TICK  FSM_TICK
#define wait_re_until_not(level,timeout_us) \
    /*计时清0*/   pir->counter=0; \
    /*读入电平*/  while(re()==level){ \
		/*等待时间片*/	  waitus(FSM_WAIT_TICK); pir->counter+=FSM_WAIT_TICK; \
		/*判断超时退出*/  if(pir->counter>timeout_us)break; \
		              }
		
//判断一个电平不在一个时间范围(min_us,max_us)
#define if_re_not_between(min_us,max_us)     if(pir->counter<min_us||pir->counter>max_us)
#define if_re_between(min_us,max_us)         if(pir->counter>=min_us&&pir->counter<=max_us)
#define if_re_higher(max_us)                 if(pir->counter>max_us)
#define if_re_lower(min_us)                  if(pir->counter<min_us)

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
	U8 io_ir;        			//红外发送io
	U8 io_re;        			//红外接收io
	U8 cmd;          			//发送命令	
	U8 wanlen;       			//要接收的数据长度
	U8 len;          			//实际接收到的数据长度
	BOOL start;      			//TRUE: 开始  FALSE:结束
	S8 state;        			//错误码		
	S32 counter;     			//计数
	U8 data[IR_DATA_MAX]; //接收数据
	U8 tmp;          			//缓存一字节
	
	////////////////////////////////
	FSM fsm;         //状态机私有变量
}IR_Type;

static IR_Type irs[IR_CHANNEL_MAX];

/*===================================================
                本地函数
====================================================*/
/*红外状态机(伪阻塞)
* 发送命令码--->读取前导码--->读入数据--->成功
* 1.状态机开始请使用 ld_ir_read_start
* 2.读取是否成功使用 ld_ir_read_isok
*/
static void ir_fsm(IR_Type*pir,FSM*fsm)
{
	fsm_time_add(FSM_TICK);//状态机时间	
	//////////////////////////////////
	Start(开始)
	{
    if(pir->start==TRUE)
			goto 发送命令码;
	}
	//////////////////////////////////
	State(发送命令码)
	{
		//高低100ms
		ir(HIGH);
		waitms(100);
		ir(LOW);
		waitms(100);
		
		//发送指令
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
		goto 读取前导码;
	}
	//////////////////////////////////
	State(读取前导码)
	{
		//等待re拉高,超时时间60ms   //实测10300
		wait_re_until_not(LOW,60000)  
		if_re_higher(60000)
			goto Header_Error;
		
	  //9ms高电平,7ms-11ms为正常,超时11ms//实测9300
		wait_re_until_not(HIGH,11000)  
		if_re_not_between(7000,11000)
			goto Header_Error;


		//4.5ms低电平,2.5ms-5ms为正常时间,超时5ms//实测4600
		wait_re_until_not(LOW,5000)  
		if_re_not_between(2500,5000)
			goto Header_Error;

		goto 读取数据;
	}
	//////////////////////////////////
	State(读取数据)
	{
		for(fsm->i=0;fsm->i<pir->wanlen;fsm->i++)
		{
			pir->tmp=0;
			for(fsm->j=0;fsm->j<8;fsm->j++)
			{
				//高电平200-600,超时600//实测300
				wait_re_until_not(HIGH,600)  
				if_re_not_between(200,600)
					goto Data_Error;

				//低电平200-1700，超时2ms
				wait_re_until_not(LOW,2000) 
				if_re_not_between(200,2000)
					goto Data_Error;
				
			  pir->tmp>>=1;
				
				//200us-600us :低电平    //实测300-400
				if_re_lower(200)
					goto Data_Error;
			
				//1200us-1700us:高电平  //实测1600
				if_re_between(1100,2000)
					pir->tmp|=0x80;//保存一位数据		
			}
			
			//读取停止码 H=200us-600us  //实测300-400
			wait_re_until_not(HIGH,600)  
			if_re_not_between(200,600)
				goto Data_Error;
			
			//读取停止码 L=700us-1400us //实测 1100-1200
			if(fsm->i!=(pir->wanlen-1))//最一个字节不读取
			{
				wait_re_until_not(LOW,1400)  		
        if_re_not_between(700,1400)
					goto Data_Error;				
			}
			//保存一个字节
			pir->data[fsm->i]=pir->tmp;
		}
		
		//接收成功
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
//	int i=0;
//	for(;i<IR_CHANNEL_MAX;i++)
//		ir_fsm(&irs[i],&irs[i].fsm);
	ir_fsm(&irs[1],&irs[1].fsm);
}

//开始读取红外数据
BOOL ld_ir_read_start(U8 ch,U8 cmd,U8 wanlen)
{
	if(ch>IR_CHANNEL_MAX)return FALSE;
	ch-=1;
	ir_lock();
	//红外已经开始读
	if(irs[ch].start==TRUE)return TRUE;
	//红外未开始读
	if(irs[ch].start==FALSE)
  {
		//开始读
		irs[ch].cmd = cmd;
		irs[ch].wanlen=wanlen;
		irs[ch].start=TRUE;
		
		//复位状态机
    memset(&irs[ch].fsm,0,sizeof(FSM));
	
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
