#include "types.h"
#include "string.h"
#include "lib.h"
#include "channel.h"
#include "dev.h"
#include "config.h"

#if  NOT_USING_IR == 0

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
extern void ld_gpio_set(U32 index,U8 value);
#define ir(data)       ld_gpio_set(io_ir,data)

//读取输入电平
#define re()           ld_gpio_get(io_re)

//等待re直到re!=level,等待时间片为tick_us,等待总超时为timeout_us
//默认等待时间片为FSM_TICK,可自行定义
#define FSM_WAIT_TICK  FSM_TICK
#define wait_re_until_not(level,timeout_us) \
    /*计时清0*/   irs.counter=0; \
    /*读入电平*/  while(re()==level){ \
		/*等待时间片*/	  waitus(FSM_WAIT_TICK); irs.counter+=FSM_WAIT_TICK; \
		/*判断超时退出*/  if(irs.counter>timeout_us)break; \
		              }
		
//判断一个电平不在一个时间范围(min_us,max_us)
#define if_re_not_between(min_us,max_us)     if((irs.counter<min_us)|| (irs.counter>max_us) )
#define if_re_between(min_us,max_us)         if((irs.counter>=min_us) && (irs.counter<=max_us) )
#define if_re_higher(max_us)                 if(irs.counter>max_us)
#define if_re_lower(min_us)                  if(irs.counter<min_us)

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
	IR_STATE state;        			//错误码		
	S32 counter;     			//计数
	U8 data[IR_DATA_MAX]; //接收数据
	U8 tmp;          			//缓存一字节
	
	BOOL inited;          //初始化标志，未初始化，状态机不能运行
	////////////////////////////////
	FSM fsm;         //状态机私有变量
}IR_Type;

U8 prebuffer[16];
static volatile IR_Type irs;
U8 afterbuffer[16];
/*===================================================
                本地函数
====================================================*/
/*红外状态机(伪阻塞)
* 发送命令码--->读取前导码--->读入数据--->成功
* 1.状态机开始请使用 ld_ir_read_start
* 2.读取是否成功使用 ld_ir_read_isok
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
		/*---------- 高低100ms --------------------- */
		ir(HIGH);
		waitms(100);
		ir(LOW);
		waitms(100);
		
		/*---------- 发送指令 --------------------- */
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
		/*---------- 读取导码 --------------------- */
		wait_re_until_not(LOW,60000)  
		if_re_higher(60000)
			goto Header_Error;
		
	  //9ms高电平,7ms-11ms为正常,超时11ms//实测9300-9500
		wait_re_until_not(HIGH,14000)  
		if_re_not_between(7000,14000)
			goto Header_Error;


		//4.5ms低电平,2.5ms-5ms为正常时间,超时5ms//实测4600
		wait_re_until_not(LOW,7000)  
		if_re_not_between(2000,7000)
			goto Header_Error;

		/*---------- 读取数据 --------------------- */

		for(fsm->i=0;fsm->i<irs.wanlen;fsm->i++)
		{
			irs.tmp=0;
			for(fsm->j=0;fsm->j<8;fsm->j++)
			{
				//高电平200-600,超时600//实测300-400-500us
				wait_re_until_not(HIGH,800)  
				if_re_not_between(200,800)
					goto Data_Error;

				//低电平200-1700，超时2ms
				wait_re_until_not(LOW,2400) 
				if_re_not_between(200,2400)
					goto Data_Error;
				
			  irs.tmp>>=1;
				
				//200us-600us :低电平    //实测300-400
				if_re_lower(200)
					goto Data_Error;
			
				//1200us-1700us:高电平  //实测1500-1600
				if_re_between(1000,2400)
					irs.tmp|=0x80;//保存一位数据		
			}
			
			//读取停止码 H=200us-600us  //实测300-400-500
			wait_re_until_not(HIGH,800)  
			if_re_not_between(200,800)
				goto Data_Error;
			
			//读取停止码 L=700us-1400us //实测 1000-1100-1200
			if(fsm->i!=(irs.wanlen-1))//最一个字节不读取
			{
				wait_re_until_not(LOW,1800)  		
        if_re_not_between(800,1800)
					goto Data_Error;				
			}
			//保存一个字节
			irs.data[fsm->i]=irs.tmp;
		}
		
		//接收成功
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
		 ld_debug_printf(1,irs.io_ir|0x80,irs.cmd ,2);//headr 读出错 type=1
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
		 ld_debug_printf(2,irs.io_ir|0x80,irs.cmd,2);//data 读出错 type=2
	#endif
	irs.counter = 0;
	irs.start=FALSE;
	irs.state=IR_Error_Data;
	irs.fsm.line=0;
	irs.fsm.save=0;
	irs.fsm.end=0;	
}

/*===================================================
                全局函数
====================================================*/
//开始读取红外数据   (ch:1-n,opposite:TRUE反向(未使用), cmd 命令, 长度)
BOOL ld_ir_read_start(U8 ir,U8 re,U8 cmd,U8 wanlen)
{
	irs.start=FALSE;
	ir_lock();
	//开始读
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

/*查看是否读完成
* return : <0：error
*        :  0: 无操作
*        :  1: 正在读
*        :  2: 读正确
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
	//正确时，弹出数据:格式化输出
	if(err==2 && dataout!= NULL)
	{
		//格式化红外数据输出
		switch(irs.cmd)
		{
			case RC_READ_ID:
			{
				U8 cs = 0xFF-cs8(irs.data,6);
				U8 i = 0;
				if(cs!=irs.data[6]){
					err=-1;
					goto END;
				}//检验失败
				memset(dataout,0,10);
				for(i=0;i<6;i++){dataout[9-i]=irs.data[i];}
			}break;
			
			case RC_READ_DATA://[0] 版本号 [1] 电量 [2] 温度 [3] 故障码 [4-5] 循环次数 [6-7] 容量 [8-9] 电芯电压 [10-11] 电流 (低位在前)
			{
				U8 cs = 0xFF-cs8(irs.data,12);
				if(cs!=irs.data[12]){
					err=-1;
					goto END;
				}//检验失败
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
			
			case RC_LOCK:         //[0]输出标志
			case RC_UNLOCK:
			case RC_UNLOCK_1HOUR:
			{
				if((0xFF-irs.data[0])!=irs.data[1])
				{
					err=-1;
					goto END;
				}//检验失败
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

/*查看红外当前故障类型:-1 ：前导码  -2:数据   0:NULL  2: 成功*/
int ld_ir_read_state(void){ return (int)irs.state;}


///*===================================================
//                测试红外读
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


