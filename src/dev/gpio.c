#include "types.h"
#include "driver_config_types.h"


static U16 hc595data=0;//���ݻ���
/*===================================================
                �����ļ�
====================================================*/
extern t_gpio_map gpio_map[];
extern const unsigned char gpio_number;
/*===================================================
                ȫ�ֺ���
====================================================*/
//��ʼ�����õ�iic�ӿ�
void ld_gpio_init(void)
{
  int i=0;
	for(;i<gpio_number;i++)
	{
		 cpu_gpio_map_config(gpio_map,i);
	}
	HC595Init();
}

//���õ�ƽ
void ld_gpio_set(U32 index,U8 value)
{
	index--;
	if(index>=gpio_number)return;
	//HC595
	if(gpio_map[index].xPort==0xFF)
	{
		if(value==0){
			hc595data&=~(1<<gpio_map[index].xPin);
		}else{
			hc595data|= (1<<gpio_map[index].xPin);
		}
		HC595Send(hc595data);
	}
	else
   cpu_gpio_map_set(gpio_map,index,value);
}

/*��ȡ��ƽ
* index :io����(1-n)
* return: 1:high  0:low
*/
U8 ld_gpio_get(U32 index)
{
	index--;
	if(index>=gpio_number)return 0;
	//HC595
	if(gpio_map[index].xPort==0xFF)
	{
			if(hc595data&(1<<gpio_map[index].xPin))return 1;
			else return 0;
	}
	else
   return cpu_gpio_map_get(gpio_map,index);
}

