/******************************************************************************
 *  Handling LTDC system (bridge between SDRAM and TFT display)
 *   - init
 *   - manipulating video buffers (visibility and active address to draw at)
 *   - switching buffers between retraces to prevent blinking
 ******************************************************************************/

#include "laa_tft_ltdc.h"
#include "laa_tft_lib.h"
#include "laa_sdram.h"
#include "laa_global_utils.h"

uint8_t  tft_LTDC_need_reload  = 0;
uint32_t tft_LTDC_retrace_time;
uint8_t  tft_LTDC_wait_retrace = 0;

extern LTDC_HandleTypeDef hltdc;

/* ~~~~ Screen layers schema ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *       (°_°) <- observer
 *  
 *  ------------------- <- Layer1 (Messages small)  
 *  ------------------- <- Layer0 (Main fullscreen) 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef struct {
  uint32_t  memoryAddr;     // Addrress of memory buffer
  uint32_t  drawAddr;       // Current address to draw
  uint16_t  width, height;  // Size of buffer in pixels
  uint16_t  posX, posY;     // Layer position at the TFT (see LTDC)
  uint16_t  clipX, clipY;   // Visible clip (inside buffer) position
  uint16_t  clipW, clipH;   // Visible clip (inside buffer) size
  uint8_t   visibleBuffer;  // 0..1 - index of visible sublayer (pointed by LTDC)
  uint8_t   activeBuffer;   // 0..1 - index of active sublayer to draw (pointed by tft_lib engine)
  uint8_t   alpha;          // blending factor (0 - transparent; 255 - opaque)
  uint8_t   setAlpha, setClipping, setPosition; // Flags to perform action after retrace
} TFT_LTDC_layer;

TFT_LTDC_layer tft_layer[2];

void tftLTDCsetupLayer(uint8_t layerIndex, uint8_t doubleBuffered);

/*  - Fill tft_main_layer and tft_msg_layer structures
 *    (some parameters are hardcoded here, some are fetching from CUBE initialization
 *  - Setup layers adresses, position, blending factor
 */
void tftLTDCuserSetup() {
  // Main layer
  TFT_LTDC_layer *L = &tft_layer[0];
  L->memoryAddr = TFT_MAIN_LAYER;
  L->posX = 0;
  L->posY = 0;
//  L->alpha = 0xFF;
  tftLTDCsetupLayer(0, 0);
  // Message layer
  L = &tft_layer[1];
  L->memoryAddr = TFT_MSG_LAYER;
  L->posX = 297;
  L->posY = 207;
//  L->alpha = 0x00;
  tftLTDCsetupLayer(1, 0);
  // Select active layer0 / GoDouble / ClearScreen / Apply settings
  tftLTDCsetActiveLayer(0);
  tftClearScreen(0);
  tftLTDCsetActiveLayer(1);
  tftClearScreen(0);
  tftLTDCsetDoubleMode(1, 1);
  tftLTDCsetDoubleMode(0, 1);
  tftLTDCsetLayerAlpha(0, 0xFF);
  tftLTDCforceReload();
  tftLTDCwaitForReload();
}

uint32_t tft_addr;  // Address of memory region to draw primitives

/*  - Setup layer address, postion, blending factor
 *  - fetch size 
 */   
void tftLTDCsetupLayer(uint8_t layerIndex, uint8_t doubleBuffered) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  L->visibleBuffer = 0;
  L->activeBuffer = doubleBuffered ? 1 : 0;
  L->width = hltdc.LayerCfg[layerIndex].ImageWidth;
  L->height = hltdc.LayerCfg[layerIndex].ImageHeight;
  L->clipX = L->clipY = 0; 
  L->clipW = L->width;
  L->clipH = L->height;
  HAL_LTDC_SetWindowPosition_NoReload(&hltdc, L->posX, L->posY, layerIndex);
  HAL_LTDC_SetAddress_NoReload(&hltdc, L->memoryAddr, layerIndex);
  tft_LTDC_need_reload = 1;
}

/* - Change layer blending factor
 */
void tftLTDCsetLayerAlpha(uint8_t layerIndex, uint8_t alpha) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  L->alpha = alpha;
  if (hltdc.LayerCfg[layerIndex].Alpha != alpha) {
    HAL_LTDC_SetAlpha_NoReload(&hltdc, alpha, layerIndex);
    tft_LTDC_need_reload = 1;
  }  
}

/* - Set Active layer (to draw primitives) - setting pointer to memory buffer
 */
void tftLTDCsetActiveLayer(uint8_t layerIndex) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  tft_addr = L->drawAddr;
  tft_h = L->height;
  tft_w = L->width;
}

/* - Set Active buffer and layer (to draw primitives) - setting pointer to memory buffer
 */
void tftLTDCsetActiveBuffer(uint8_t layerIndex, uint8_t buferIndex) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  uint8_t isActive = L->drawAddr == tft_addr;
  L->activeBuffer = buferIndex;
  L->drawAddr = L->memoryAddr + L->activeBuffer * L->width * L->height * TFT_PS;
  if (isActive) {
    buferIndex = L->activeBuffer;
  }  
}

/* - Set LTDC layer adress
 */
void tftLTDCsetAddress(uint8_t layerIndex) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  uint32_t addr = L->memoryAddr +
     (L->visibleBuffer * L->width * L->height + 
      L->clipY * L->width +
      L->clipX) * TFT_PS;
  HAL_LTDC_SetAddress_NoReload(&hltdc, addr, layerIndex);
  tft_LTDC_need_reload = 1;
}

/* - Set swap active and visible buffers
 */
void tftLTDCswapBuffers(uint8_t layerIndex) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  if (L->activeBuffer == L->visibleBuffer) return;
  tftLTDCsetActiveBuffer(layerIndex, !L->activeBuffer);
  L->visibleBuffer = !L->visibleBuffer;
  tftLTDCsetAddress(layerIndex);
}

/* - Set double / single buffer mode
 */
void tftLTDCsetDoubleMode(uint8_t layerIndex, uint8_t doubleMode) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  if (doubleMode) {
    if (L->activeBuffer != L->visibleBuffer) return;
    tftLTDCsetActiveBuffer(layerIndex, !L->visibleBuffer);
  } else {
    if (L->activeBuffer == L->visibleBuffer) return;
    tftLTDCsetActiveBuffer(layerIndex, L->visibleBuffer);
  }
}  

/* - Clip subarea from buffer
 * TODO - noreload mode or synchronized mode
 */
void tftLTDCsetClipping(uint8_t layerIndex, int16_t x, int16_t y, int16_t w, int16_t h) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  L->clipX = x;
  L->clipY = y;
  L->clipW = w;
  L->clipH = h;
  tftLTDCsetAddress(layerIndex);
  HAL_LTDC_SetWindowSize(&hltdc, w, h, layerIndex);
  HAL_LTDC_SetPitch(&hltdc, L->width, layerIndex);
}  

/* - Clip subarea from buffer
 */
void tftLTDCsetPosition(uint8_t layerIndex, int16_t x, int16_t y) {
  TFT_LTDC_layer *L = &tft_layer[layerIndex];
  L->posX = x;
  L->posY = y;
  HAL_LTDC_SetWindowPosition_NoReload(&hltdc, x, y, layerIndex);
  HAL_LTDC_SetPitch_NoReload(&hltdc, L->width, layerIndex);
  tft_LTDC_need_reload = 1;
}  

/* - Init waiting for retrace to reoad LTDC parameters
 */
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

/* - Wait for reload (blocking mode)
 */
void tftLTDCwaitForReload() {
  while (tftLTDCisWaitingForReload()) {};
}

/* - Wait for reload (non-blocking mode)
 */
uint8_t tftLTDCisWaitingForReload() {
  if (!tft_LTDC_wait_retrace) return 0;
  if (tft_LTDC_retrace_time - HAL_GetTick() <= 100) return 1;
  tft_LTDC_wait_retrace = 0;
  return 0;
}

/* - Clear wait_for_retrace flag
 */
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
  tft_LTDC_wait_retrace = 0;
}  
