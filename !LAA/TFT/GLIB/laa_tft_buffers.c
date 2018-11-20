#include "laa_tft_buffers.h"
#include "laa_tft_lib.h"
#include "laa_tft_led.h"
#include "laa_sdram.h"

extern uint32_t tft_addr;  // Address of memory region to draw primitives
uint8_t  tft_draw_buffer = 0;
uint8_t  tft_show_buffer = 0;
uint32_t tft_retrace_time;
uint8_t  tft_wait_retrace = 0;

extern LTDC_HandleTypeDef hltdc;

void tftInit() {
  tftClearScreen(0xff0000);
  HAL_LTDC_SetAddress(&hltdc, TFT_SCREEN, 0);  
  tftGoDoubleBuffered();
  HAL_Delay(50);
  HAL_LTDC_SetAlpha(&hltdc, 0xFF, 0);
  HAL_Delay(50);
  tftLEDinit(0, 192);
}  
  
void tftSetDrawAddress() {
  tft_addr = TFT_SCREEN + tft_draw_buffer * TFT_BUFFER_SIZE;
}  

void tftGoDoubleBuffered() {
  tft_draw_buffer = tft_show_buffer == 0;
  tftSetDrawAddress();
}

void tftGoSingleBuffered() {
  tft_draw_buffer = tft_show_buffer != 0;
  tftSetDrawAddress();
}

void tftNextFrame() {
  if (tft_draw_buffer == tft_show_buffer) {
    return;
  }  
  tft_show_buffer = tft_show_buffer == 0;
  tft_draw_buffer = !tft_show_buffer;
  tftSetDrawAddress();
  HAL_LTDC_SetAddress_NoReload(&hltdc,
    TFT_SCREEN + tft_show_buffer * TFT_BUFFER_SIZE,
  0);  
  tft_wait_retrace = 1;
  tft_retrace_time = HAL_GetTick() + 100;
  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
}  

/* - Wait for reload (blocking mode)
 */
void tftWaitForReload() {
  while (tftIsWaitingForReload()) {};
}

/* - Wait for reload (non-blocking mode)
 */
uint8_t tftIsWaitingForReload() {
  if (!tft_wait_retrace) return 0;
  if (tft_retrace_time - HAL_GetTick() <= 100) return 1;
  tft_wait_retrace = 0;
  return 0;
}

/* - Clear wait_for_retrace flag
 */
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
  tft_wait_retrace = 0;
}
