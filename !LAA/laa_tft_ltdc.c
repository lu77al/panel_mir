/******************************************************************************
 *  Handling LTDC system (bridge between SDRAM and TFT display)
 *   - init
 *   - manipulating video buffers (visibility and active address to draw at)
 *   - switching buffers between retraces to prevent blinking
 ******************************************************************************/

#include "laa_tft_ltdc.h"
#include "laa_sdram.h"
#include "laa_global_utils.h"

extern LTDC_HandleTypeDef hltdc;

/* ~~~~ Screen layers schema ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *       (°_°) <- observer
 *  
 *  ------------------- <- Layer1   Alpha = 0 -> transparent / Alpha = 255 -> opaque
 *  ------------------- <- Layer0   Alpha = 255 -> opaque (always)
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

uint8_t  tft_vis_buf_index;       
uint8_t  tft_active_buf_index;        
uint8_t  tft_wait_for_retrace_cnt;   // Flag to wait with drawing in memory

uint32_t tft_addr;  // Address of memory region to draw primitives

/* Command LTDC to make buffer visible after vertical retrace */
void tftLTDCsetVisibleBuffer(uint8_t index) {
  if (tft_vis_buf_index == index) return;
  index &= 1; // correct range
  tft_vis_buf_index = index;
  HAL_LTDC_SetAddress_NoReload(&hltdc, index ? TFT_MAIN_BUF_1 : TFT_MAIN_BUF_0, 0);
  tft_wait_for_retrace_cnt = 2;
  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
} 

/* Set active buffer and corresponding address in memory  */
void tftLTDCsetActiveBuffer(uint8_t index) {
  if (tft_active_buf_index == index) return;
  index &= 1; // correct range
  tft_active_buf_index = index;
  tft_addr = index ? TFT_MAIN_BUF_1 : TFT_MAIN_BUF_0;
}

/* Invert or swap active and visible buffers */
void tftLTDCswapLayers() {
  if (tft_active_buf_index == tft_vis_buf_index) return;
  tftLTDCsetActiveBuffer(!tft_active_buf_index);
  tftLTDCsetVisibleBuffer(!tft_vis_buf_index);
}

/* Select double/single buffer mode */
void tftLTDCsetDoubleMode(uint8_t state) {
  if (state == ON) { // Double (Active != Visible)
    if (tft_active_buf_index != tft_vis_buf_index) return;
    tftLTDCsetActiveBuffer(!tft_vis_buf_index);
  } else { // Single (Active == Visible)
    if (tft_active_buf_index == tft_vis_buf_index) return;
    tftLTDCsetActiveBuffer(tft_vis_buf_index);
  }  
}  

/* Set LTDC layer's addresses, ... */
void tftLTDCinit(uint8_t doubleMode) {
  tft_wait_for_retrace_cnt = 0;
  HAL_LTDC_SetAddress(&hltdc, TFT_MAIN_BUF_0, 0);   // LTDC layers' adresses
  HAL_LTDC_SetAddress(&hltdc, TFT_MSG_BUF, 1);
  tft_vis_buf_index = 0;
  if (doubleMode) {
    tft_active_buf_index =  1;
    tft_addr = TFT_MAIN_BUF_1;
  } else {
    tft_active_buf_index =  0;
    tft_addr = TFT_MAIN_BUF_0;
  }  
  HAL_Delay(50);
}

/* Clear wait flag if retrace interrupt wasn't successful
 *   - start it from task manager */
void tftLTDCdismissWaitRetrace() {
  __disable_irq();
  if (tft_wait_for_retrace_cnt) tft_wait_for_retrace_cnt--;
  __enable_irq();
}  

/* Clear wait flag after retrace */
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
  tft_wait_for_retrace_cnt = 0;
}  
