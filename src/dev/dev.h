#ifndef __DEV_H__
#define __DEV_H__
#include "types.h"


/*===================================================
                gpio
====================================================*/
extern void ld_gpio_init(void);
extern void ld_gpio_set(U32 index,U8 value);
extern U8 ld_gpio_get(U32 index);


#endif

