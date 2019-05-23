#ifndef __AL_CRC16_H__
#define __AL_CRC16_H__
#include "stm32f7xx_hal.h"

void add_byte_to_crc16(uint8_t *CRC16, uint8_t B);
void init_crc16(uint8_t *CRC16);

#endif // __AL_CRC16_H__
