#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"




typedef struct{

	volatile U8 addr485;          //485��ַ
  volatile U16 software_version;//������


}System;

extern System system;

#endif
