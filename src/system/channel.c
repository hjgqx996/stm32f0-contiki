
/*===================================================
功能:与仓道操作相关函数
====================================================*/
#include "includes.h"

/*===================================================
                配置文件
====================================================*/

/*===================================================
                宏，类型
====================================================*/
/*仓道充电宝编号是否为NULL*/
const U8 null_id[10] = {0,0,0,0,0,0,0,0,0,0};
#define channel_id_is_not_null(id) buffer_cmp((U8*)null_id,id,10);

static Channel chs[CHANNEL_MAX];
/*===================================================
                本地函数
====================================================*/


/*===================================================
                全局函数
====================================================*/

/*获取仓道数据
*channel:1-n
*/
Channel*channel_data_get(U8 channel)
{
	channel-=1;
	if(channel>=CHANNEL_MAX)return NULL;
	return &chs[channel];
}

/*----------------------------------
检测
-----------------------------------*/



/*----------------------------------
仓道运行状态
仓道告警
仓道错误
-----------------------------------*/
void channel_state_check(U8 ch)
{

}

void channel_warn_check(U8 ch)
{

}

void channel_error_check(U8 ch)
{

}

/*----------------------------------
仓道充电
仓道断电
-----------------------------------*/
void channel_charge(U8 ch)
{


}
void channel_discharge(U8 ch)
{
  //仓道断电 
	
	//从排队的列表中删除本仓道
//	queue_delete(ch);
}

/*----------------------------------
   读仓道数据
-----------------------------------*/
void channel_read_data(U8 ch)
{

	//根据选择，使用iic 还是 ir 读取数
	//使用iic(阻塞)
	
	//iic失败几次->使用ir
	
	//使用ir(非阻塞)
	
	//ir失败几次->使用iic


	/////////////////////////////////////
	//读取成功，处理一下数据



  /////////////////////////////////////
}