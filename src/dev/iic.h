#ifndef __IIC_H__
#define __IIC_H__

#include "types.h"

typedef struct{
	 GPIO_Map io[2]; //iic��IO�˿�����
	 U8 dir;            //���� dir=0: sda=io[0] scl=io[1]     dir=1:sda=io[1] scl=io[0]				   
}LD_IIC_MAP;



#endif


