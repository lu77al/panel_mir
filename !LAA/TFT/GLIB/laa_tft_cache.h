#ifndef __AL_TFT_CACHE_H__
#define __AL_TFT_CACHE_H__

#include "stm32f7xx_hal.h"

#define TFT_ROM_CACHE_PTR  0x20000L
#define TFT_ROM_CACHE (FLASH_BASE + TFT_ROM_CACHE_PTR)

uint8_t *tftFindObject(const char *id);
uint8_t *tftLocateCache(uint32_t size, const char* id);
void    tftClearCache();


#endif // __AL_TFT_CACHE_H__

