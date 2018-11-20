#ifndef __AL_TFT_BUFFERS_H__
#define __AL_TFT_BUFFERS_H__

#include "stm32f7xx_hal.h"

void tftInit();
void tftGoDoubleBuffered();
void tftGoSingleBuffered();
void tftNextFrame();
void tftWaitForReload();
uint8_t tftIsWaitingForReload();

#endif // __AL_TFT_LIB_H__
