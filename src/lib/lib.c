#include "lib.h"



/**
	* �Ƚ����������Ƿ����
	* return :TRUE �����  FALSE:����
  */
BOOL buffer_cmp(U8* pbuf,volatile U8* pbuf1, U8 len)
{
  while(len--)
  {
    if(*pbuf != *pbuf1)
    {
      return FALSE;
    }
    pbuf++;
    pbuf1++;
  }
  return TRUE;
}
