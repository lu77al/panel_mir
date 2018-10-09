/******************************************************************************
 *  Handling LTDC system (bridge between SDRAM and TFT display)
 *   - init
 *   - manipulating layers (visibility and active address to draw at)
 *   - switching layers between retraces to prevent blinking
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

uint8_t  tft_visible_layer;       
uint8_t  tft_active_layer;        
uint8_t  tft_wait_for_retrace_cnt;   // Flag to wait with drawing in memory

uint32_t tft_addr;  // Address of memory region to draw primitives

/* Command LTDC to make layer visible after vertical retrace */
void tftLTDCsetVisibleLayer(uint8_t layer) {
  if (tft_visible_layer == layer) return;
  layer &= 1; // correct range
  tft_visible_layer = layer;
  HAL_LTDC_SetAlpha_NoReload(&hltdc, layer ? 255 : 0, 1);
  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
  tft_wait_for_retrace_cnt = 2;
}  

/* Set active layer and corresponding address in memory  */
void tftLTDCsetActiveLayer(uint8_t layer) {
  if (tft_active_layer == layer) return;
  layer &= 1; // correct range
  tft_active_layer = layer;
  tft_addr = layer ? TFT_LAYER1 : TFT_LAYER0;
}

/* Invert or swap active and visible layers */
void tftLTDCswapLayers() {
  tftLTDCsetActiveLayer(!tft_active_layer);
  tftLTDCsetVisibleLayer(!tft_visible_layer);
}

/* Select double/single layer mode */
void tftLTDCsetDoubleMode(uint8_t state) {
  if (state == ON) { // Double (Active != Visible)
    if (tft_active_layer != tft_visible_layer) return;
    tftLTDCsetActiveLayer(!tft_visible_layer);
  } else { // Single (Active == Visible)
    if (tft_active_layer == tft_visible_layer) return;
    tftLTDCsetActiveLayer(tft_visible_layer);
  }  
}  

/* Set LTDC layer's addresses, ... */
void tftLTDCinit() {
  tft_wait_for_retrace_cnt = 0;
  HAL_LTDC_SetAddress(&hltdc, TFT_LAYER0, 0);   // LTDC layers' adresses
  HAL_LTDC_SetAddress(&hltdc, TFT_LAYER1, 1);
  HAL_LTDC_SetAlpha(&hltdc, 255, 0);            // Bouth opaque
  HAL_LTDC_SetAlpha(&hltdc, 255, 1);
  tft_visible_layer = 1;
  tft_active_layer =  0;     // to force setSetActive to update address        
  tftLTDCsetActiveLayer(1);  // Visible == Active -> single layer mode
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
