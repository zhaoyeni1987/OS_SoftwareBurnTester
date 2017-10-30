#pragma once
/*
* crc.h
*
*  Created on: 2017Äê7ÔÂ10ÈÕ
*      Author: Administrator
*/

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
/************************** Constant Definitions *****************************/

#define CRC16_INIT  0x0000

/************************** Function Prototypes ******************************/

unsigned short CalCRC16(void *pData, unsigned int dwNumOfBytes);

unsigned int CalCRC32(unsigned char* pData, int dwNumOfBytes);

void Insert16CRC_U8(unsigned char* pu8Start, unsigned int u32Length, unsigned short* pu16CrcAddr);

#ifdef __cplusplus
}
#endif
