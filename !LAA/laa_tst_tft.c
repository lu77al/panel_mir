/******************************************************************************
 *  Test blocks for TFT group
 ******************************************************************************/
#include "laa_tft_lib.h"
#include "laa_tst_tft.h"

void tftTest_1() {
  tftClearScreen(0x000055);

  tftSetForeground(0x00AA00);
  tftRect(100, 50, TFT_W - 200, TFT_H - 100);
  tftSetForeground(0xFF0000);
  tftRect(200, 100, TFT_W - 400, TFT_H - 200);

  tftSetFont("F16X32.FNT");
  tftSetTextPos(10, 10);
  tftSetForeground(0xFFFF00);
  tftSetTextTransparency(1);
  tftPrint("Text#1 ", 255);

  tftSetFont("KC10X16.FNT");
  tftSetBackground(0x004400);
  tftSetTextTransparency(0);
  tftPrint("Text#2 ", 255);

  tftSetFont("F16X32.FNT");
  tftSetTextPos(400, -14);
  tftSetForeground(0xFFFFFF);
  tftSetTextTransparency(1);
  tftPrint("Text#3 ", 255);
  
  tftSetTextTransparency(0);
  tftPrint("Text#4 ", 4);
}  
  
/*
  uint32_t s_addr = tft_addr + TFT_PS * (y * TFT_W + x);
  uint32_t i_addr = (uint32_t)bmp_addr + TFT_PS * (bmp_y * bmp_w + bmp_x);
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = TFT_W - w;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].InputOffset = bmp_w - w;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) return;
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) return;
  if (HAL_DMA2D_Start(&hdma2d, i_addr, s_addr, w, h) == HAL_OK) {
    if (tft_wait_dma) HAL_DMA2D_PollForTransfer(&hdma2d, 50);
  }  
*/

extern uint32_t tft_addr;  // Address of memory region to draw primitives
extern DMA2D_HandleTypeDef hdma2d;

void tftTest_2() {
  tftClearScreen(0x000055);
// --- Prepare source test region
  tftSetForeground(0x000000); // -> Converts to 0x0000 (high bit Alpha = 0 - transparent)
  tftRect(0, 0, 300, 300);
  tftSetForeground(0x80FFFF);
  tftRect(50, 50, 200, 200);
  tftSetForeground(0x000000);
  tftRect(75, 75, 150, 150);
  tftSetForeground(0x80FF00);
  tftRect(100, 100, 100, 100);
// --- Prepare destination test region
  for (uint8_t i = 0; i < 10; i++) {
    uint32_t color = 0xFF00FF + i * 0x1BE4;
    tftRect(300, i*30, 300, 30);
  }  
// --- Prepare DMA2D
  uint32_t s_addr = tft_addr;
  uint32_t d_addr = tft_addr + 2 * 300;
  uint32_t offset = TFT_W - 300;
// output  
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = offset;
// foreground layer  
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB1555;
  hdma2d.LayerCfg[1].InputOffset = offset;
// background layer  
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0xFF;
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[0].InputOffset = offset;
// start transfer  
  HAL_DMA2D_BlendingStart(&hdma2d, s_addr, d_addr, d_addr, 300, 300);
  HAL_DMA2D_PollForTransfer(&hdma2d, 200);
}
