#include "stm32f0xx.h"
#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>
#include "types.h"
#include "time.h"
static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

extern int channel_read_delay_ms;

void   SysTick_Handler(void)
{
  current_clock++;
  if(etimer_pending() && etimer_next_expiration_time() <= current_clock) {
    etimer_request_poll();
    /* printf("%d,%d\n", clock_time(),etimer_next_expiration_time  	()); */

  }
  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
		{
			extern void channel_check_timer_1s();
			channel_check_timer_1s();//1秒一次
		}
  }
	//循环读延时
	if(channel_read_delay_ms>0)
		channel_read_delay_ms -=1000/CLOCK_SECOND;
	if(channel_read_delay_ms<0)
		channel_read_delay_ms=0;
	
	
	//系统灯
	{
		extern void ld_system_led_timer(int ms);
	  ld_system_led_timer(10);
	}
	
	//仓道灯闪
	{
		extern void channels_les_flash_timer(int timer_ms);
		channels_les_flash_timer(1000/CLOCK_SECOND);
	}
	
	//充电状态机计时
	{
		extern void charge_fms_timer(int ms);
		charge_fms_timer(1000/CLOCK_SECOND);
	}
	
	//弹仓事件
	{
		extern void channel_lock_timer(int ms);
		channel_lock_timer(1000/CLOCK_SECOND);
	}
}

/*初始化系统定时器*/
void clock_init()
{	
	RCC_ClocksTypeDef RCC_Clocks;
	/* SysTick end of count event each 10ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	if (SysTick_Config(RCC_Clocks.HCLK_Frequency /CLOCK_SECOND)) 
	{ 
    	while(1); 
	} 
}



clock_time_t clock_time(void)
{
  return current_clock;
}

/*当前时间ms*/
time_t time(time_t*t)
{
	time_t tt = current_clock *(1000/CLOCK_SECOND);
	if(t!=NULL)*t=tt;
	return tt;
}

#if 0
/* The inner loop takes 4 cycles. The outer 5+SPIN_COUNT*4. */

#define SPIN_TIME 2 /* us */
#define SPIN_COUNT (((MCK*SPIN_TIME/1000000)-5)/4)

#ifndef __MAKING_DEPS__

void clock_delay(unsigned int t)
{
#ifdef __THUMBEL__ 
  asm volatile("1: mov r1,%2\n2:\tsub r1,#1\n\tbne 2b\n\tsub %0,#1\n\tbne 1b\n":"=l"(t):"0"(t),"l"(SPIN_COUNT));
#else
#error Must be compiled in thumb mode
#endif
}
#endif
#endif /* __MAKING_DEPS__ */

unsigned long clock_seconds(void)
{
  return current_seconds;
}
