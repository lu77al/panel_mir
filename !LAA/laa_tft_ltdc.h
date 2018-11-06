#ifndef __AL_TFT_LTDC_H__
#define __AL_TFT_LTDC_H__

#include "stm32f7xx_hal.h"

typedef struct {
  uint32_t  memoryAddr;     // Addrress of memory buffer
  uint16_t  width, height;  // Size of buffer in pixels
  uint16_t  posX, posY;     // Layer position at the TFT (see LTDC)
  uint16_t  clipX, clipY;   // Visible clip (inside buffer) position
  uint16_t  clipW, clipH;   // Visible clip (inside buffer) size
  uint8_t   ltdcIndex;      // 0..1 - corresponding LTDC layer index
  uint8_t   visibleBuffer;  // 0..1 - index of visible sublayer (pointed by LTDC)
  uint8_t   activeBuffer;   // 0..1 - index of active sublayer to draw (pointed by tft_lib engine)
  uint8_t   alpha;          // blending factor (0 - transparent; 255 - opaque)
} TFT_LTDC_layer;

extern TFT_LTDC_layer tft_main_layer;
extern TFT_LTDC_layer tft_msg_layer;

void tftLTDCuserSetup();
void tftLTDCinitLayer(TFT_LTDC_layer *layer, uint8_t doubleBuffered);
void tftLTDCsetLayerAlpha(TFT_LTDC_layer *layer, uint8_t alpha);
void tftLTDCsetActiveLayer(TFT_LTDC_layer *layer);
void tftLTDCsetActiveBuffer(TFT_LTDC_layer *layer, uint8_t buferIndex);
void tftLTDCsetVisibleBuffer(TFT_LTDC_layer *layer, uint8_t buferIndex);
void tftLTDCswapBuffers(TFT_LTDC_layer *layer);
void tftLTDCsetDoubleMode(TFT_LTDC_layer *layer, uint8_t doubleMode);
void tftLTDCforceReload();
void tftLTDCwaitForReload();
uint8_t tftLTDCisWaitingForReload();

#endif // __AL_TFT_LTDC_H__

