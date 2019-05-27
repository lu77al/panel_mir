#ifndef __AL_INTERFACE_H__
#define __AL_INTERFACE_H__

#include "stm32f7xx_hal.h"

extern void (*uiDrawScreenRoutine)();
extern void (*uiNextKeyRoutine)(uint8_t key);

extern uint8_t uiNeedUpdate;

void uiDrawScreen();
void uiProcessUserInput();

#endif // __AL_INTERFACE_H__

