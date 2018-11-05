/******************************************************************************
 *  Test blocks for TFT group
 ******************************************************************************/
#include "laa_tft_lib.h"
#include "laa_tst_tft.h"
#include "laa_sdram.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

void tftTestFonts() {
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

void tftTest_simple_copy() {
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
    tftSetForeground(color);
    tftRect(300, i*30, 300, 30);
  }  
// --- Prepare DMA2D
  uint32_t s_addr = tft_addr;
  uint32_t d_addr = tft_addr + 2 * 300;
  uint32_t offset = TFT_W - 300;
// output  
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = offset;
// foreground layer  
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].InputOffset = offset;
// Apply configuration
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
    asm("nop");
    return;
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) {
    asm("nop");
    return;
  }  
// start transfer  
  if (HAL_DMA2D_Start(&hdma2d, s_addr, d_addr, 300, 300) != HAL_OK) {
    asm("nop");
    return;
  }
    
  HAL_DMA2D_PollForTransfer(&hdma2d, 200);
}

void tftTest_blending_copy() {
  tftClearScreen(0x000055);
// --- Prepare source test region
  tftSetForeground(0x000000); // -> Converts to 0x0000 (high bit Alpha = 0 - transparent)
  tftRect(0, 0, 250, 250);
  tftSetForeground(0x80FFFF);
  tftRect(50, 50, 150, 150);
  tftSetForeground(0x000000);
  tftRect(75, 75, 100, 100);
  tftSetForeground(0x80FF00);
  tftRect(100, 100, 50, 50);
// --- Prepare destination test region
  for (uint8_t i = 0; i < 50; i++) {
    uint32_t color = 0x8000FF + i * 0x0500 - i * 5;
    tftSetForeground(color);
    tftRect(250, i*5, 250, 5);
  }  
// --- Prepare DMA2D
  uint32_t offset = TFT_W - 250;
// output  
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = offset;
// foreground layer  
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0x40;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB1555;
  hdma2d.LayerCfg[1].InputOffset = offset;
// background layer  
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0xFF;
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[0].InputOffset = offset;
// Apply configuration
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
    asm("nop");
    return;
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) {
    asm("nop");
    return;
  }  
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 0) != HAL_OK) {
    asm("nop");
    return;
  }  
// start transfer  
  uint32_t fg_addr = tft_addr;
  uint32_t bg_addr = tft_addr + 2 * 250;
//  uint32_t dst_addr = tft_addr + 2 * 500;
  
  if (HAL_DMA2D_BlendingStart(&hdma2d, fg_addr, bg_addr, bg_addr, 250, 250) != HAL_OK) {
    asm("nop");
    return;
  }  
  HAL_DMA2D_PollForTransfer(&hdma2d, 200);
}

#define IMG_SIZE 60
#define BALL_CNT 45

int16_t ballX[BALL_CNT]; //  = 100;
int16_t ballY[BALL_CNT]; //  = 0;
int16_t ballDX[BALL_CNT]; // = 7;
int16_t ballDY[BALL_CNT]; // = 11;
int16_t ballSize = 25;

int16_t msgX  = 300;
int16_t msgY  = 10;
int16_t msgDX = -3;
int16_t msgDY = 5;
int16_t msgW = 200;
int16_t msgH = 100;

extern RNG_HandleTypeDef hrng;

void tstInitBalls() {
//int r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.  
  for (uint8_t i = 0; i < BALL_CNT; i++) {
    ballX[i] = HAL_RNG_GetRandomNumber(&hrng) % (TFT_W - IMG_SIZE);
    ballY[i] = HAL_RNG_GetRandomNumber(&hrng) % (TFT_H - IMG_SIZE);
    ballDX[i] = HAL_RNG_GetRandomNumber(&hrng) % 23 - 11;
    ballDY[i] = HAL_RNG_GetRandomNumber(&hrng) % 23 - 11;
  }  
}  

void tftMoveAxis(int16_t *pos, int16_t*speed, int16_t max) {
  *pos += *speed;
  if (*pos > max) {
    *pos = max * 2 - *pos;
    *speed = -*speed;
  } else if (*pos < 0) {
    *pos = -*pos;
    *speed = -*speed;
  }  
}  

uint16_t image[IMG_SIZE * IMG_SIZE]; // ARGB1555

void tstPrepareImg() {
  memset(image, 0, sizeof(image));
  for (int16_t r = 0; r < (IMG_SIZE + 1) / 2; r++) {
    int16_t d = IMG_SIZE - 1 - r * 2;
    int16_t w = (int16_t)round((sqrt(IMG_SIZE * IMG_SIZE - d * d)));
    uint16_t *top = &image[r * IMG_SIZE + (IMG_SIZE - w) / 2];
    uint16_t *bot = &image[(IMG_SIZE - 1 - r) * IMG_SIZE + (IMG_SIZE - w) / 2];
    uint16_t color = 31 * (IMG_SIZE / 2 - r) / IMG_SIZE;
    for (; w; w--) {
      if (w % 2) color += 32;
      *(top++) = 0xC000 + color;
      *(bot++) = 0xC000 + color; //0x909F;
    }  
  }  
}  

void tstDrawBall(uint16_t x, uint16_t y) {
  // --- Prepare DMA2D
  uint32_t offset = TFT_W - IMG_SIZE;
// output  
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = offset;
// foreground layer  
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB1555;
  hdma2d.LayerCfg[1].InputOffset = 0;
// background layer  
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0xFF;
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[0].InputOffset = offset;
// Apply configuration
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
    asm("nop");
    return;
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) {
    asm("nop");
    return;
  }  
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 0) != HAL_OK) {
    asm("nop");
    return;
  }  
// start transfer  
  uint32_t fg_addr = (uint32_t)image;
  uint32_t bg_addr = tft_addr + 2 * (y * TFT_W + x);
//  uint32_t dst_addr = tft_addr + 2 * 500;
  
  if (HAL_DMA2D_BlendingStart(&hdma2d, fg_addr, bg_addr, bg_addr, IMG_SIZE, IMG_SIZE) != HAL_OK) {
    asm("nop");
    return;
  }  
  HAL_DMA2D_PollForTransfer(&hdma2d, 200);
}

void tftDrawLayer0() {
  for (uint16_t i = 0; i < 48; i++) {
    uint32_t color = 0xFF0020 - (i << 16) * 5 + (i << 8) * 5;
    tftSetForeground(color);
    tftRect(0, i * 10, 800, 10);
  }  

  
  for (uint8_t i = 0; i < BALL_CNT; i++) {
    tstDrawBall(ballX[i], ballY[i]);
    tftMoveAxis(&ballX[i], &ballDX[i], TFT_W - IMG_SIZE);
    tftMoveAxis(&ballY[i], &ballDY[i], TFT_H - IMG_SIZE);
  }  

  
  
  
//  tftSetForeground(0x4040FF);
//  tftRect(ballX, ballY, ballSize, ballSize);
//  tftMoveAxis(&ballX, &ballDX, TFT_W - ballSize);
//  tftMoveAxis(&ballY, &ballDY, TFT_H - ballSize);
}  

extern LTDC_HandleTypeDef hltdc;
extern uint8_t  tft_wait_for_retrace_cnt;

void tftSwitchLayers() {
  tft_wait_for_retrace_cnt = 1;
  HAL_LTDC_SetAddress_NoReload(&hltdc, tft_addr, 0);
  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
  while (tft_wait_for_retrace_cnt) {
    asm("nop");
  }
  tft_addr = (tft_addr ==  TFT_LAYER0) ? TFT_LAYER1 : TFT_LAYER0;
}  

void tftSwitchLayerAdressTest() {
  uint16_t *pixel = (uint16_t *)TFT_LAYER_TOP;
  for (int32_t i = 0; i < 200*100; i++) {
    *(pixel++) = 0xFFFF;
  }  
  for (int32_t i = 30; i < 70; i++) {
    for (int32_t j = 30; j < 170; j++) {
        *((uint16_t *)TFT_LAYER_TOP + i*200 + j) = 0;
    }    
  }  
  
  tstPrepareImg();

  tstInitBalls();
  
  
  tft_addr = TFT_LAYER0;
  tftDrawLayer0();
  HAL_LTDC_SetAddress(&hltdc, TFT_LAYER0, 0);
  HAL_LTDC_SetAddress(&hltdc, TFT_LAYER_TOP, 1);
  HAL_LTDC_SetAlpha(&hltdc, 255, 0);
  HAL_LTDC_SetAlpha(&hltdc, 128, 1);
  
  while (1) {
    tftSwitchLayers();
    tftDrawLayer0();

    tftMoveAxis(&msgX, &msgDX, TFT_W - msgW);
    tftMoveAxis(&msgY, &msgDY, TFT_H - msgH);

    HAL_LTDC_SetWindowPosition_NoReload(&hltdc, msgX, msgY, 1);
    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
    while (tft_wait_for_retrace_cnt) {
      asm("nop");
    }
  }  
}  


