#ifndef __AL_TFT_LTDC_H__
#define __AL_TFT_LTDC_H__

#include "stm32f7xx_hal.h"

void tftLTDCsetVisibleLayer(uint8_t layer);
void tftLTDCsetActiveLayer(uint8_t layer);
void tftLTDCswapLayers();
void tftLTDCsetDoubleMode(uint8_t state);
void tftLTDCinit();
void tftLTDCdismissWaitRetrace();

#endif // __AL_TFT_LTDC_H__

