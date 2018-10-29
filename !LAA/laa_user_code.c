/******************************************************************************
 *  Code protected from CubeMX
 ******************************************************************************/
#include "laa_user_code.h"
#include "laa_sdram.h"
#include "laa_sdcard.h"
#include "laa_tft_led.h"
#include "laa_tft_ltdc.h"
#include "laa_tft_lib.h"
#include "laa_tst_tft.h"


//extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim10;
//extern UART_HandleTypeDef huart6;
//extern UART_HandleTypeDef huart1;
//extern LTDC_HandleTypeDef hltdc;
//extern DMA2D_HandleTypeDef hdma2d;
//extern RNG_HandleTypeDef hrng;

volatile uint16_t main_tic_cnt = 0;

void userInit() {
  sdramInit();                  // Activate SDRAM 
  tftLTDCinit();                // Init LTDC layers
  tftClearScreen(0);            // Black screen to start with
  HAL_Delay(100);
  tftLEDinit(0, 192);           // Activate TFT backlight
  HAL_TIM_Base_Start(&htim10);  // Time source for task dispatcher (flags only)
  sdMount();                    // Mount SD card
  tftResetObjects();            // Clear cache ...
}  
   
   

void userMain() {
  userInit();
  
  tftTest_1();
//  tftTest_2();
  
  while (1) {

// --- 500 Hz TIM10 driven processes ---
    if (__HAL_TIM_GET_FLAG(&htim10, TIM_FLAG_UPDATE) != RESET) {
      __HAL_TIM_CLEAR_IT(&htim10, TIM_IT_UPDATE);

      main_tic_cnt++;
      
      if ((main_tic_cnt &= 0x7f) == 0x20) {           // 3.90625 Hz
      } else if ((main_tic_cnt &= 0x3f) == 0x10) {    // 7.8125  Hz #1
        tftLTDCdismissWaitRetrace();
      } else if ((main_tic_cnt &= 0x3f) == 0x30) {    // 7.8125  Hz #2
      } else if ((main_tic_cnt &= 0x07) == 0x02) {    // 62.5    Hz
        tftLightAdjust();
      }  
      
    }
  }
}  

   