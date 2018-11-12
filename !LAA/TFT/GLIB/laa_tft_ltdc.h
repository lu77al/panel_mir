#ifndef __AL_TFT_LTDC_H__
#define __AL_TFT_LTDC_H__

#include "stm32f7xx_hal.h"

void tftLTDCuserSetup();
void tftLTDCsetLayerAlpha(uint8_t layerIndex, uint8_t alpha);
void tftLTDCsetActiveLayer(uint8_t layerIndex);
void tftLTDCswapBuffers(uint8_t layerIndex);
void tftLTDCsetDoubleMode(uint8_t layerIndex, uint8_t doubleMode);
void tftLTDCsetClipping(uint8_t layerIndex, int16_t x, int16_t y, int16_t w, int16_t h);
void tftLTDCsetPosition(uint8_t layerIndex, int16_t x, int16_t y);
void tftLTDCforceReload();
void tftLTDCwaitForReload();
uint8_t tftLTDCisWaitingForReload();

#endif // __AL_TFT_LTDC_H__

