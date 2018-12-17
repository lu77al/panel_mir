/******************************************************************************
 *  Code protected from CubeMX
 ******************************************************************************/
#include "laa_user_code.h"
#include "laa_sdram.h"
#include "laa_sdcard.h"
#include "laa_tft_led.h"
#include "laa_tft_lib.h"
#include "laa_tft_buffers.h"
#include "laa_tst_tft.h"
#include "laa_scr_tasks.h"

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
  HAL_TIM_Base_Start(&htim10);  // Time source for task dispatcher (flags only)
  sdMount();                    // Mount SD card
  tftInit();
}  
  

void userMain() {
  userInit();

//  tftSwitchLayerAdressTest();
  
//  tftTest_simple_copy();
//  tftTest_blending_copy();

  tftSetWaitDMA(0);
  tftGoDoubleBuffered();
  scrSetNewDataFlag();
/*  
uint8_t scrNeedNewContent();
void scrResetPnt(uint8_t mark);
void scrSaveMark(uint8_t mark);
void scrSetNewDataFlag();

void scrSetBG(uint32_t color);
void scrSetFG(uint32_t color);
void scrBar(int16_t x, int16_t y, int16_t w, int16_t h);
*/
  
  uint16_t theSize = 0;
  
  while (1) {
    scrPerformNextTask();
    if (scrNeedNewContent()) {
      scrResetPnt(0);
      scrSetBG(0x0000ff);
      scrBar(0, 0, 800, 480);
      scrSetBG(0x0000ff);
      scrBar(0, 0, 800, 480);
      scrSetBG(0xff0000);
      uint8_t k = theSize < 250 ? 5 + theSize : 505 - theSize;
      uint16_t w = 800 * k / 255;
      uint16_t h = 480 * k / 255;
      scrBar((800 - w) / 2, (480 - h) / 2, w, h);
      theSize = (theSize + 1) % 500;
      scrSetNewDataFlag();
    }  
    
    
    

// --- 500 Hz TIM10 driven processes ---
    if (__HAL_TIM_GET_FLAG(&htim10, TIM_FLAG_UPDATE) != RESET) {
      __HAL_TIM_CLEAR_IT(&htim10, TIM_IT_UPDATE);

      main_tic_cnt++;
      
      if ((main_tic_cnt &= 0x7f) == 0x20) {           // 3.90625 Hz
      } else if ((main_tic_cnt &= 0x3f) == 0x10) {    // 7.8125  Hz #1
//        tftLTDCdismissWaitRetrace();
      } else if ((main_tic_cnt &= 0x3f) == 0x30) {    // 7.8125  Hz #2
      } else if ((main_tic_cnt &= 0x07) == 0x02) {    // 62.5    Hz
        tftLightAdjust();
      }  
      
    }
  }
}  

   