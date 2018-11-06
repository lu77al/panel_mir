/******************************************************************************
 *  Handling LTDC system (bridge between SDRAM and TFT display)
 *   - init
 *   - manipulating video buffers (visibility and active address to draw at)
 *   - switching buffers between retraces to prevent blinking
 ******************************************************************************/

#include "laa_tft_ltdc.h"
#include "laa_sdram.h"
#include "laa_global_utils.h"

#define TFT_PS  2

uint8_t  tft_LTDC_need_reload  = 0;
uint32_t tft_LTDC_retrace_time;
uint8_t  tft_LTDC_wait_retrace = 0;

extern LTDC_HandleTypeDef hltdc;

/* ~~~~ Screen layers schema ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *       (°_°) <- observer
 *  
 *  ------------------- <- Layer1   Alpha = 0 -> transparent / Alpha = 255 -> opaque
 *  ------------------- <- Layer0   Alpha = 255 -> opaque (always)
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TFT_LTDC_layer tft_main_layer;
TFT_LTDC_layer tft_msg_layer;

void tftLTDCuserSetup() {

  tft_main_layer.ltdcIndex = 0;
  tft_main_layer.memoryAddr = TFT_MAIN_LAYER;
  tft_main_layer.width = 800;
  tft_main_layer.height = 480;
  tft_main_layer.posX = 0;
  tft_main_layer.posY = 0;
  tft_main_layer.alpha = 0xFF;

  tftLTDCinitLayer(&tft_main_layer, 1);
  
  tft_msg_layer.ltdcIndex = 1;
  tft_msg_layer.memoryAddr = TFT_MSG_LAYER;
  tft_msg_layer.width = 205;
  tft_msg_layer.height = 65;
  tft_msg_layer.posX = 297;
  tft_msg_layer.posY = 207;
  tft_msg_layer.alpha = 0x35;

  tftLTDCinitLayer(&tft_msg_layer, 1);
}

uint32_t tft_addr;  // Address of memory region to draw primitives

/* Setup LTDC address, size, postion; default parameters
 *   Prepare before call: ltdcIndex, memoryAddr, width, height, posX, posY, alpha
 */   
void tftLTDCinitLayer(TFT_LTDC_layer *layer, uint8_t doubleBuffered) {
  TFT_LTDC_layer *L = layer;
  HAL_LTDC_SetAddress(&hltdc, L->memoryAddr, L->ltdcIndex);   // LTDC layers' adress
  HAL_LTDC_SetWindowSize(&hltdc, L->width, L->height, L->ltdcIndex);
  HAL_LTDC_SetWindowPosition(&hltdc, L->posX, L->posY, L->ltdcIndex);
  HAL_LTDC_SetAlpha(&hltdc, L->alpha, L->ltdcIndex);
  layer->visibleBuffer = 0;
  layer->activeBuffer = doubleBuffered ? 1 : 0;
  L->clipX = L->clipY = 0; 
  L->clipW = L->width;
  L->clipH = L->height;
}

void tftLTDCsetLayerAlpha(TFT_LTDC_layer *layer, uint8_t alpha) {
  layer->alpha = alpha;
  HAL_LTDC_SetAlpha_NoReload(&hltdc, alpha, layer->ltdcIndex);
  tft_LTDC_need_reload = 1;
}

void tftLTDCsetActiveLayer(TFT_LTDC_layer *layer) {
  TFT_LTDC_layer *L = layer;
  tft_addr = L->memoryAddr + L->activeBuffer * L->width * L->height * TFT_PS;
}

void tftLTDCsetActiveBuffer(TFT_LTDC_layer *layer, uint8_t buferIndex) {
  layer->activeBuffer = buferIndex;
  tftLTDCsetActiveLayer(layer);
}

void tftLTDCsetVisibleBuffer(TFT_LTDC_layer *layer, uint8_t buferIndex) {
  TFT_LTDC_layer *L = layer;
  L->visibleBuffer = buferIndex;
  uint32_t addr = L->memoryAddr +
     (L->visibleBuffer * L->width * L->height + 
      L->clipY * L->width +
      L->clipX) * TFT_PS;
  HAL_LTDC_SetAddress_NoReload(&hltdc, addr, L->ltdcIndex);
  tft_LTDC_need_reload = 1;
}

void tftLTDCswapBuffers(TFT_LTDC_layer *layer) {
  if (layer->activeBuffer == layer->visibleBuffer) return;
  tftLTDCsetActiveBuffer(layer, !layer->activeBuffer);
  tftLTDCsetVisibleBuffer(layer, !layer->visibleBuffer);
}

void tftLTDCsetDoubleMode(TFT_LTDC_layer *layer, uint8_t doubleMode) {
  TFT_LTDC_layer *L = layer;
  if (doubleMode) {
    if (L->activeBuffer != L->visibleBuffer) return;
    tftLTDCsetActiveBuffer(layer, !L->visibleBuffer);
  } else {
    if (L->activeBuffer == L->visibleBuffer) return;
    tftLTDCsetActiveBuffer(layer, L->visibleBuffer);
  }
}  

void tftLTDCforceReload() {
  if (tft_LTDC_need_reload) {
    tft_LTDC_need_reload = 0;
    tft_LTDC_wait_retrace = 1;
    tft_LTDC_retrace_time = HAL_GetTick() + 100;
    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
  } else {
    tft_LTDC_wait_retrace = 0;
  }  
}  

void tftLTDCwaitForReload() {
  while (tftLTDCisWaitingForReload()) {};
}

uint8_t tftLTDCisWaitingForReload() {
  if (!tft_LTDC_wait_retrace) return 0;
  if (tft_LTDC_retrace_time - HAL_GetTick() <= 100) return 1;
  tft_LTDC_wait_retrace = 0;
  return 0;
}

void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
  tft_LTDC_wait_retrace = 0;
}  

//uint8_t  tft_vis_buf_index;
//uint8_t  tft_active_buf_index;        


///* Command LTDC to make buffer visible after vertical retrace */
//void tftLTDCsetVisibleBuffer(uint8_t index) {
//  if (tft_vis_buf_index == index) return;
//  index &= 1; // correct range
//  tft_vis_buf_index = index;
//  HAL_LTDC_SetAddress_NoReload(&hltdc, index ? TFT_MAIN_BUF_1 : TFT_MAIN_LAYER, 0);
//  tft_wait_for_retrace_cnt = 2;
//  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
//} 
//
///* Set active buffer and corresponding address in memory  */
//void tftLTDCsetActiveBuffer(uint8_t index) {
//  if (tft_active_buf_index == index) return;
//  index &= 1; // correct range
//  tft_active_buf_index = index;
//  tft_addr = index ? TFT_MAIN_BUF_1 : TFT_MAIN_LAYER;
//}
//
///* Invert or swap active and visible buffers */
//void tftLTDCswapLayers() {
//  if (tft_active_buf_index == tft_vis_buf_index) return;
//  tftLTDCsetActiveBuffer(!tft_active_buf_index);
//  tftLTDCsetVisibleBuffer(!tft_vis_buf_index);
//}

/* Select double/single buffer mode */
//void tftLTDCsetDoubleMode(uint8_t state) {
//  if (state == ON) { // Double (Active != Visible)
//    if (tft_active_buf_index != tft_vis_buf_index) return;
//    tftLTDCsetActiveBuffer(!tft_vis_buf_index);
//  } else { // Single (Active == Visible)
//    if (tft_active_buf_index == tft_vis_buf_index) return;
//    tftLTDCsetActiveBuffer(tft_vis_buf_index);
//  }  
//}  

///* Set LTDC layer's addresses, ... */
//void tftLTDCinit(uint8_t doubleMode) {
////  tft_wait_for_retrace_cnt = 0;
//  HAL_LTDC_SetAddress(&hltdc, TFT_MAIN_LAYER, 0);   // LTDC layers' adresses
//  HAL_LTDC_SetAddress(&hltdc, TFT_MSG_LAYER, 1);
//  tft_vis_buf_index = 0;
//  if (doubleMode) {
//    tft_active_buf_index =  1;
//    tft_addr = TFT_MAIN_BUF_1;
//  } else {
//    tft_active_buf_index =  0;
//    tft_addr = TFT_MAIN_LAYER;
//  }  
//  HAL_Delay(50);
//}

/* Clear wait flag if retrace interrupt wasn't successful
 *   - start it from task manager */
//void tftLTDCdismissWaitRetrace() {
//  __disable_irq();
//  if (tft_wait_for_retrace_cnt) tft_wait_for_retrace_cnt--;
//  __enable_irq();
//}  

/* Clear wait flag after retrace */
