#ifndef __AL_TFT_LED_H__
#define __AL_TFT_LED_H__

#include "stm32f7xx_hal.h"

void tftLEDinit(uint8_t start_val, uint8_t target_val);
void tftLEDsetInst(uint8_t newval);
void tftLEDsetTarget(uint8_t newval);
void tftLightAdjust();

#endif // __AL_TFT_LED_H__

