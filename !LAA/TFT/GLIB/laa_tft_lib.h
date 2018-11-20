#ifndef __AL_TFT_LIB_H__
#define __AL_TFT_LIB_H__

#include "stm32f7xx_hal.h"

#define TFT_PIXEL  2
#define TFT_WIDTH  800
#define TFT_HEIGHT 480
#define TFT_BUFFER_SIZE (TFT_WIDTH * TFT_HEIGHT * TFT_PIXEL)

void tftSetWaitDMA(uint8_t mode);
void tftRect(int16_t x, int16_t y, uint16_t w, uint16_t h);
void tftSetForeground(uint32_t color);
void tftSetBackground(uint32_t color);
void tftClearScreen(uint32_t color);
void tftResetObjects();
void tftSetFont(const char* name);
void tftSetTextPos(int16_t x, int16_t y);
void tftSetTextTransparency(int8_t tr);
void tftPrint(char *text, uint8_t length);

#endif // __AL_TFT_LIB_H__

