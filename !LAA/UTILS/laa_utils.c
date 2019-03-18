#include "laa_utils.h"

void laaWritePin(LAA_GPIO pin, uint8_t state) {
  HAL_GPIO_WritePin(pin.Port, pin.Pin, (GPIO_PinState)(state));
}  
  
uint8_t laaReadPin(LAA_GPIO pin) {
  return HAL_GPIO_ReadPin(pin.Port, pin.Pin);
}  
