#ifndef __AL_UTILS_H__
#define __AL_UTILS_H__
#include "stm32f7xx_hal.h"

#define OFF    0
#define ON     1

#define MEM16(addr) (*(uint8_t *)(addr) | (*(uint8_t *)(addr + 1) << 8))
#define MEM32(addr) (*(uint8_t *)(addr) | (*(uint8_t *)(addr + 1) << 8) | (*(uint8_t *)(addr + 2) << 16) | (*(uint8_t *)(addr + 3) << 24))

#define SAVE_MEM16(addr, val) {*(uint8_t *)(addr) = val; *(uint8_t *)(addr + 1) = val >> 8;}

#define SWAP16(a,b) {int16_t t=a; a=b; b=t;}
#define SWAP32(a,b) {int32_t t=a; a=b; b=t;}

typedef struct { // GPIO_Pin
  GPIO_TypeDef*  Port;
  uint16_t       Pin;
} LAA_GPIO;

void    laaWritePin(LAA_GPIO pin, uint8_t state);
uint8_t laaReadPin(LAA_GPIO pin);

#endif // __AL_UTILS_H__
