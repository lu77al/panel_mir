/******************************************************************************
 *  Code protected from CubeMX
 ******************************************************************************/
#include "laa_user_code.h"
#include "laa_sdram.h"
#include "laa_sdcard.h"
#include "laa_tft_lib.h"
#include "laa_tft_buffers.h"
#include "laa_tst_tft.h"
#include "laa_scr_tasks.h"
#include "laa_keyboard.h"
#include "laa_setup.h"
#include "laa_interface.h"
#include "laa_components.h"

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
  kbdInit();
  
//  stpInitSetup();
}  

void tstKeyRur(uint8_t key) {
  if (key == KEY_ENTER) {
    cmpLogMemoNextLine();
  } else {
    cmpLogMemoPrintColor(" Red",  0xff0000);
    cmpLogMemoPrintColor(" Green", 0x00ff00);
    cmpLogMemoPrintColor(" Blue",  0x0000ff);
  }
}
  
void userMain() {
  userInit();

//  tstPrepareBackground();

/*  
  uiDrawScreenRoutine = stpShowActiveMenu;
  uiNextKeyRoutine = stpMenuInput;
  uiNeedUpdate = 1; 
*/
  uiNextKeyRoutine = tstKeyRur;

  cmpLogMemoInit("���� LOG_MEMO", "�� �������� �� ����������");
  
  while (1) {
    scrPerformNextTask();

    uiProcessUserInput();
   
    uiDrawScreen();

// --- 500 Hz TIM10 driven processes ---
    if (__HAL_TIM_GET_FLAG(&htim10, TIM_FLAG_UPDATE) != RESET) {
      __HAL_TIM_CLEAR_IT(&htim10, TIM_IT_UPDATE);

      main_tic_cnt++;
      kbdScan();
      
      if ((main_tic_cnt & 0x7f) == 0x20) {           // 3.90625 Hz
      } else if ((main_tic_cnt & 0x3f) == 0x10) {    // 7.8125  Hz #1
        cmpLogMemoForceUpdate();
      } else if ((main_tic_cnt & 0x3f) == 0x30) {    // 7.8125  Hz #2
      } else if ((main_tic_cnt & 0x07) == 0x02) {    // 62.5    Hz
      }  
      
    }
  }
}  

   