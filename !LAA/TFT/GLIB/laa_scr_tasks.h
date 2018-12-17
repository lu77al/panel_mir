#ifndef __AL_SCR_TASKS_H__
#define __AL_SCR_TASKS_H__

#include "stm32f7xx_hal.h"

void scrPerformNextTask();
uint8_t scrNeedNewContent();
void scrResetPnt(uint8_t mark);
void scrSaveMark(uint8_t mark);
void scrSetNewDataFlag();

void scrSetBG(uint32_t color);
void scrSetFG(uint32_t color);
void scrBar(int16_t x, int16_t y, int16_t w, int16_t h);


#endif // __AL_SCR_TASKS_H__

