/******************************************************************************
 *  Code protected from CubeMX
 ******************************************************************************/
#include "laa_user_code.h"
#include "laa_sdram.h"
#include "laa_tft_led.h"
#include "laa_tft_ltdc.h"


//extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
//extern UART_HandleTypeDef huart6;
//extern UART_HandleTypeDef huart1;
//extern LTDC_HandleTypeDef hltdc;
//extern DMA2D_HandleTypeDef hdma2d;
//extern RNG_HandleTypeDef hrng;

volatile uint16_t main_tic_cnt = 0;

void user_main() {
  HAL_TIM_Base_Start(&htim10);  // Time source for task dispatcher (flags only)
  sdram_init();                 // Activate SDRAM 
  tft_led_init(0, 192);         // Activate TFT backlight
  tft_ltdc_init();              // Init LTDC layers
  
  while (1) {

// --- 500 Hz TIM10 driven processes ---
    if (__HAL_TIM_GET_FLAG(&htim10, TIM_FLAG_UPDATE) != RESET) {
      __HAL_TIM_CLEAR_IT(&htim10, TIM_IT_UPDATE);

      main_tic_cnt++;
      
      if ((main_tic_cnt &= 0x7f) == 0x20) {           // 3.90625 Hz
      } else if ((main_tic_cnt &= 0x3f) == 0x10) {    // 7.8125  Hz #1
        tft_ltdc_pass_wait_for_retrace();
      } else if ((main_tic_cnt &= 0x3f) == 0x30) {    // 7.8125  Hz #2
      } else if ((main_tic_cnt &= 0x07) == 0x02) {    // 62.5    Hz
        tft_light_adjust();
      }  
      
    }
  }
}  

   