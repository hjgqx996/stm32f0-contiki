#ifndef __SYSTEM_H__
#define __SYSTEM_H__
#include "types.h"




typedef struct{

	volatile U8 addr485;          //485µØÖ·
  volatile U16 software_version;//Èí¼þ±àºÅ


}System;

extern System system;

#endif
