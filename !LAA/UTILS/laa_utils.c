#include "laa_utils.h"
#include "ctype.h"

void laaWritePin(LAA_GPIO pin, uint8_t state) {
  HAL_GPIO_WritePin(pin.Port, pin.Pin, (GPIO_PinState)(state));
}  
  
uint8_t laaReadPin(LAA_GPIO pin) {
  return HAL_GPIO_ReadPin(pin.Port, pin.Pin);
}  

void laaStrToUpper(char *str) {
  char *ch = str;
  while (*ch) {
    *ch = toupper(*ch);
    ch++;
  }
}

uint16_t laaGet16(void *mem) { // Get 16bit value from memory regardless alignment
  uint16_t val;
  ((uint8_t *)&val)[0] = ((uint8_t *)mem)[0];
  ((uint8_t *)&val)[1] = ((uint8_t *)mem)[1];
  return val;
}  

uint32_t laaGet24(void *mem) { // Get 24bit value from memory regardless alignment
  uint32_t val;
  ((uint8_t *)&val)[0] = ((uint8_t *)mem)[0];
  ((uint8_t *)&val)[1] = ((uint8_t *)mem)[1];
  ((uint8_t *)&val)[2] = ((uint8_t *)mem)[2];
  ((uint8_t *)&val)[3] = 0;
  return val;
}  

uint32_t laaGet32(void *mem) { // Get 32bit value from memory regardless alignment
  uint32_t val;
  ((uint8_t *)&val)[0] = ((uint8_t *)mem)[0];
  ((uint8_t *)&val)[1] = ((uint8_t *)mem)[1];
  ((uint8_t *)&val)[2] = ((uint8_t *)mem)[2];
  ((uint8_t *)&val)[3] = ((uint8_t *)mem)[3];
  return val;
}  

void laaSet24(void *mem, uint32_t val) { // Set 24bit value to memory regardless alignment
  ((uint8_t *)mem)[0] = ((uint8_t *)&val)[0];
  ((uint8_t *)mem)[1] = ((uint8_t *)&val)[1];
  ((uint8_t *)mem)[2] = ((uint8_t *)&val)[2];
}  
