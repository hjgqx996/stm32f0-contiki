
#include "stm32f0xx.h"
#include "iic.h"

/*===================================================
                iic≈‰÷√
====================================================*/
LD_IIC_MAP ld_iic_maps[]={
   //iic1 sda - scl  :pb8 pb9
   {
    {
	0,8,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3,
    0,9,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3
	},
	0
   },
   //iic2 sda - scl  :pb6 pb7
   {
   	{
   	0,6,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3,
    0,7,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3
	},
	0
   },

   //iic3 sda - scl  :pb4 pb5
   {
    {
	 0,4,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3,
     0,5,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3
	 },
	0
   },

   //iic4 sda - scl  :pb2 pb3
   {
    {
	 0,2,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3,
     0,3,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3
	 },
	0
   },

   //iic5 sda - scl  :pb11 pb12
   { 
    {
	0,11,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3,
    0,12,GPIO_Mode_OUT,GPIO_OType_OD,GPIO_PuPd_NOPULL,GPIO_Speed_Level_3
	},
	0
   },
};

