#include "laa_tft_led.h"
extern TIM_HandleTypeDef htim3;     // Timer for backlight PWM

#define TFT_LIGHT_RANGE UC_LEDLIGHT_PERIOD // Timer period (full range of PWM)
#define TFT_LIGHT_BOTTOM    10  // % of full range
#define TFT_LIGHT_TOP       90  // % of full range

uint8_t  tft_light_inst;    // Instant value of backlight (0..255)
uint8_t  tft_light_target;  // Point adjust to ...

/* Apply selected brightness of TFT backlight (setup TIM3 PWM)
 *  tft_light_inst = 0   -> TFT_LIGHT_BOTTOM %
 *  tft_light_inst = 255 -> TFT_LIGHT_TOP %
 */
void tftLEDapply() {
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,
   TFT_LIGHT_RANGE * TFT_LIGHT_BOTTOM / 100 +
   (((TFT_LIGHT_RANGE * (TFT_LIGHT_TOP - TFT_LIGHT_BOTTOM) / 100) * tft_light_inst) >> 8));
}

/* Init hardware (TIM3) set initial backlight values */
void tftLEDinit(uint8_t start_val, uint8_t target_val) {
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  // Turn on backlight PWM
  tft_light_inst   = start_val;
  tft_light_target = target_val;
  tftLEDapply();
  HAL_Delay(100);
  HAL_GPIO_WritePin(LCD_BL_DSBL_GPIO_Port, LCD_BL_DSBL_Pin, GPIO_PIN_RESET); // Enable backlight
}

/* Immediate change of TFT backlight 0..255  */
void tftLEDsetInst(uint8_t newval) {
  tft_light_inst = tft_light_target = newval;
  void tft_led_apply();
}

/* Set target value of TFT backlight 0..255 (adjust to it smoothly) */
void tftLEDsetTarget(uint8_t newval) {
  tft_light_target = newval;
}

/* Call it from main tact. Perfoms smooth change of TFT backlight */
void tftLightAdjust() {
  if (tft_light_inst == tft_light_target) return;
  if (tft_light_inst < tft_light_target) tft_light_inst++; else tft_light_inst--;
  void tft_led_apply();
}
