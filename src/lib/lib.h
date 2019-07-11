#ifndef __LIB_H__
#define __LIB_H__
#include "types.h"


/**
	* 比较两个数组是否相等
	* return :TRUE ：相等  FALSE:不等
  */
BOOL buffer_cmp(U8* pbuf,volatile U8* pbuf1, U8 len);

#endif