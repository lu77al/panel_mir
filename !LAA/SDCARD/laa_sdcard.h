#ifndef __AL_SDCARD_H__
#define __AL_SDCARD_H__

#include "stm32f7xx_hal.h"

void    sdMount();
uint8_t sdOk();
void    sdSetCurDir(char *dir);
uint8_t sdOpenForRead(const char *name);
uint8_t sdOpenForWrite(const char *name);
uint8_t sdRead(uint8_t *buffer, uint32_t size);
uint8_t sdWrite(uint8_t *buffer, uint32_t size);

//uint8_t sdSeek(uint32_t pos);
uint8_t sdClose();

#endif // __AL_SDCARD_H__

