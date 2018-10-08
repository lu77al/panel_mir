#ifndef __AL_TFT_LIB_H__
#define __AL_TFT_LIB_H__

#include "stm32f7xx_hal.h"

#define TFT_W     800
#define TFT_H     480

void tftSetWaitDMA(uint8_t mode);
void tftRect(int16_t x, int16_t y, uint16_t w, uint16_t h);
void tftSetForeground(uint32_t color);
void tftSetBackground(uint32_t color);

#endif // __AL_TFT_LIB_H__

