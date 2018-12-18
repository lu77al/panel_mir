#ifndef __AL_SCR_TASKS_H__
#define __AL_SCR_TASKS_H__

#include "stm32f7xx_hal.h"

void scrResetPnt(uint8_t mark);
void scrSaveMark(uint8_t mark);
void scrSetNewDataFlag();
uint8_t scrNeedNewContent();
void scrPerformNextTask();

void scrGoDouble();
void scrGoSingle();
void scrSetBG(uint32_t color);
void scrSetFG(uint32_t color);
void scrCLS(int32_t color);
void scrBar(int16_t x, int16_t y, int16_t w, int16_t h);
void scrSetPenWidth(int8_t width);
void scrSetPenPattern(int32_t pattern);
void scrMoveTo(int16_t x, int16_t y);
void scrLineTo(int16_t x, int16_t y);
void scrLineRel(int16_t x, int16_t y);
void scrLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void scrSetFontStatic(char *name);
void scrSetFontDynamic(char *name);
void scrSetTextTransparency(int8_t transparent);
void scrSetTextPos(int16_t x, int16_t y);
void scrTextOutStatic(void *text, uint8_t maxLen);
void scrTextOutDynamic(void *text, uint8_t maxLen);
void scrInitPoly(uint8_t filled, uint8_t closed);;
void scrPolyVertex(int16_t x, int16_t y);
void scrEllipse(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t s, uint16_t e, uint8_t filled, uint8_t closed);
void scrSetBMPstatic(char *name, uint32_t trColor888);
void scrSetBMPdynamic(char *name, uint32_t trColor888);
void scrDrawBMP(int16_t x, int16_t y, uint8_t alpha);

#endif // __AL_SCR_TASKS_H__

