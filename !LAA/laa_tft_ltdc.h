#ifndef __AL_TFT_LTDC_H__
#define __AL_TFT_LTDC_H__

#include "stm32f7xx_hal.h"

void tftLTDCsetVisibleBuffer(uint8_t layer);
void tftLTDCsetActiveBuffer(uint8_t layer);
void tftLTDCswapLayers();
void tftLTDCsetDoubleMode(uint8_t state);
void tftLTDCinit(uint8_t doubleMode);
void tftLTDCdismissWaitRetrace();

#endif // __AL_TFT_LTDC_H__

