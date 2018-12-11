#ifndef __AL_TFT_LIB_H__
#define __AL_TFT_LIB_H__

#include "stm32f7xx_hal.h"
#include "laa_tft_buffers.h"

#define TFT_PIXEL  2
#define TFT_WIDTH  800
#define TFT_HEIGHT 480
#define TFT_BUFFER_SIZE (TFT_WIDTH * TFT_HEIGHT * TFT_PIXEL)

//*********** COLORS + GENERAL ROUTINES **************
void tftSetWaitDMA(uint8_t mode);
void tftSetForeground(uint32_t color);
void tftSetBackground(uint32_t color);
void tftClearScreen(uint32_t color);
void tftSetPenWidth(uint8_t w);
void tftSetPenPattern(uint32_t pattern);
void tftResetObjects();

//*********** BASIC PRIMITIVES **************
void tftRect(int16_t x, int16_t y, uint16_t w, uint16_t h);
void tftMoveTo(int16_t x, int16_t y);
void tftLineTo(int16_t x, int16_t y);
void tftLineRel(int16_t x, int16_t y);
void tftLine(int16_t x1, int16_t y1, uint16_t x2, uint16_t y2);

uint8_t tftPloyProcess();
void tftPolyInit(uint8_t filled);
void tftPolyAddVertex(int16_t x, int16_t y);

//*********** FONT AND TEXT ROUTINES **************
void tftSelectFont(const char* name);
void tftDrawChar(char ch);
void tftPrint(char *text, uint8_t length);
void tftSetTextPos(int16_t x, int16_t y);
void tftSetTextTransparency(int8_t tr);
//*********** BITMAPS **************
void tftSelectBMP(const char* name, uint32_t trColor);
void tftDrawBMP(int16_t x, int16_t y);

#endif // __AL_TFT_LIB_H__

