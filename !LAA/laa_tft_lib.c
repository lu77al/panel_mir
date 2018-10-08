#include "laa_tft_lib.h"
#include "laa_sdram.h"

extern DMA2D_HandleTypeDef hdma2d;
extern uint32_t tft_addr;      // Address of memory region to draw primitives

// --- TFT AND PIXEL SIZE ---
#define TFT_W     800
#define TFT_H     480
#define TFT_PS    2

uint8_t   tft_wait_dma = 1;
uint32_t  tft_fg_color = 0xffffff;
uint32_t  tft_bg_color = 0x000000;

void tft_rect(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  if ((w | h) == 0) return;
  if (x >= TFT_W) return;
  if (y >= TFT_H) return;
  if ((x + w) < 1) return;
  if ((y + h) < 1) return;
  if ((x + w) > TFT_W) w = TFT_W - x;
  if ((y + h) > TFT_H) h = TFT_H - y;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  uint32_t addr = tft_addr + TFT_PS * (y * TFT_W + x);
  hdma2d.Init.Mode = DMA2D_R2M;
  hdma2d.Init.OutputOffset = TFT_W - w;
  if (HAL_DMA2D_Init(&hdma2d) == HAL_OK) {
    if (HAL_DMA2D_Start(&hdma2d, tft_fg_color, addr, w, h) == HAL_OK) {
      if (tft_wait_dma) HAL_DMA2D_PollForTransfer(&hdma2d, 200);
    }  
  }  
}
