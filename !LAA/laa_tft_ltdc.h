#ifndef __AL_TFT_LTDC_H__
#define __AL_TFT_LTDC_H__

#include "stm32f7xx_hal.h"

void tft_ltdc_set_visible_layer(uint8_t layer);
void tft_ltdc_set_active_layer(uint8_t layer);
void tft_ltdc_swap_layers();
void tft_ltdc_go_double();
void tft_ltdc_go_single();
void tft_ltdc_init();
void tft_ltdc_pass_wait_for_retrace();

#endif // __AL_TFT_LTDC_H__

