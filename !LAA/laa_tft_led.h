#ifndef __AL_TFT_LED_H__
#define __AL_TFT_LED_H__

#include "stm32f7xx_hal.h"

void tft_led_init(uint8_t start_val, uint8_t target_val);
void tft_led_set_inst(uint8_t newval);
void tft_led_set_target(uint8_t newval);
void tft_light_adjust();

#endif // __AL_TFT_LED_H__

