#ifndef __AL_PROJ_STRUCTURE_H__
#define __AL_PROJ_STRUCTURE_H__

#include "stm32f7xx_hal.h"

#define PROJECT         ((uint8_t *)POJECT_ADDR)
#define PROJECT_SIZE    laaGet24(PROJECT)

void     prjClear();
uint8_t  *prjCRC();
uint8_t  prjSave();

#endif // __AL_PROJ_STRUCTURE_H__

