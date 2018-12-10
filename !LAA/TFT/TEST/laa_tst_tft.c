/******************************************************************************
 *  Test blocks for TFT group
 ******************************************************************************/
#include "laa_tft_lib.h"
#include "laa_tst_tft.h"
#include "laa_sdram.h"
#include "laa_tft_led.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"

/*   
void tftTestFonts() {
  tftClearScreen(0x000055);

  tftSetForeground(0x00AA00);
  tftRect(100, 50, TFT_WIDTH - 200, TFT_HEIGHT - 100);
  tftSetForeground(0xFF0000);
  tftRect(200, 100, TFT_WIDTH - 400, TFT_HEIGHT - 200);

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
*/
   
/*
  uint32_t s_addr = tft_addr + TFT_PS * (y * TFT_WIDTH + x);
  uint32_t i_addr = (uint32_t)bmp_addr + TFT_PS * (bmp_y * bmp_w + bmp_x);
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = TFT_WIDTH - w;
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

/*   
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
  uint32_t offset = TFT_WIDTH - 300;
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
*/
   
/*   
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
  uint32_t offset = TFT_WIDTH - 250;
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
*/

extern uint32_t tft_addr;  // Address of memory region to draw primitives
extern DMA2D_HandleTypeDef hdma2d;
   
#define IMG_SIZE 60
#define BALL_CNT 120

int16_t ballX[BALL_CNT]; //  = 100;
int16_t ballY[BALL_CNT]; //  = 0;
int16_t ballDX[BALL_CNT]; // = 7;
int16_t ballDY[BALL_CNT]; // = 11;
int16_t ballSize = 25;

int16_t msgX  = 300;
int16_t msgY  = 10;
int16_t msgDX = -1;
int16_t msgDY = 2;
int16_t msgW = 205;
int16_t msgH = 65;
int16_t margin = 0;
int16_t factor = 0;
int16_t dMargin = 1;
int16_t dFactor = 1;

extern RNG_HandleTypeDef hrng;

void tstInitBalls() {
//int r = rand();      // Returns a pseudo-random integer between 0 and RAND_MAX.  
  HAL_RNG_Init(&hrng);
  for (uint8_t i = 0; i < BALL_CNT; i++) {
    ballX[i] = HAL_RNG_GetRandomNumber(&hrng) % (TFT_WIDTH - IMG_SIZE);
    ballY[i] = HAL_RNG_GetRandomNumber(&hrng) % (TFT_HEIGHT - IMG_SIZE);
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
  uint32_t offset = TFT_WIDTH - IMG_SIZE;
// output  
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = offset;
// foreground layer  
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0x60;
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
  uint32_t bg_addr = tft_addr + 2 * (y * TFT_WIDTH + x);
// uint32_t dst_addr = tft_addr + 2 * 500;
   
  if (HAL_DMA2D_BlendingStart(&hdma2d, fg_addr, bg_addr, bg_addr, IMG_SIZE, IMG_SIZE) != HAL_OK) {
    asm("nop");
    return;
  }  
  HAL_DMA2D_PollForTransfer(&hdma2d, 200);
}

extern LTDC_HandleTypeDef hltdc;

void tftDrawLayer0() {
  for (uint16_t i = 0; i < 48; i++) {
    uint32_t color = 0xFF0020 - (i << 16) * 5 + (i << 8) * 5;
    tftSetBackground(color);
    tftRect(0, i * 10, 800, 10);
  }  
  
  for (uint8_t i = 0; i < BALL_CNT; i++) {
    tstDrawBall(ballX[i], ballY[i]);
    tftMoveAxis(&ballX[i], &ballDX[i], TFT_WIDTH - IMG_SIZE);
    tftMoveAxis(&ballY[i], &ballDY[i], TFT_HEIGHT - IMG_SIZE);
  }  
 
}  

//void tftSwitchLayers() {
//  tft_wait_for_retrace_cnt = 1;
//  HAL_LTDC_SetAddress_NoReload(&hltdc, tft_addr, 0);
//  tst_LTDC_need_reload = true;
//  tft_addr = (tft_addr ==  TFT_MAIN_LAYER) ? TFT_MAIN_BUF_1 : TFT_MAIN_LAYER;
//}  
//
//void tstWaitForReload() {
//  if (!tst_LTDC_need_reload) return;
//  tst_LTDC_need_reload = false;
//  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
//  tft_wait_for_retrace_cnt = 1;
//  while (tft_wait_for_retrace_cnt) {}
//}

/****************************
LTDC.background - bottom background
Layer.background - color out of active window
Layer.Alpha / BlendingFactor1? - active window blending   
Layer.Alpha0 / BlendingFactor2? - background blending   
*****************************/  

extern uint8_t tft_pen_img[5000];

void tftTestDMA2D_A4() {
  asm("nop");
  asm("nop");
  asm("nop");
  
  uint8_t img[5000]; // 100x100
  memset(img, 0, 5000); // Clear
  for (uint8_t i = 0; i<100; i++) {
    memset(&img[i*50], 0xff, (100 - i) / 2);
    if (i % 2) img[i*50 + (100 - i) / 2] = 0x0f;
  }
  // --- Prepare DMA2D
  uint32_t offset = TFT_WIDTH - 100;
// output  
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = offset;
// foreground layer  
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A4;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0x8000ffff;
  hdma2d.LayerCfg[1].InputOffset = 0;
// background layer  
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0xFF;
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
  uint32_t fg_addr = (uint32_t)img;
  uint32_t bg_addr = tft_addr + 2 * (50 * TFT_WIDTH + 50);
// uint32_t dst_addr = tft_addr + 2 * 500;
   
  if (HAL_DMA2D_BlendingStart(&hdma2d, fg_addr, bg_addr, bg_addr, 100, 100) != HAL_OK) {
    asm("nop");
    return;
  }  
  HAL_DMA2D_PollForTransfer(&hdma2d, 200);

/*  

  // --- Prepare DMA2D
  uint32_t offset = TFT_WIDTH - 20;
// output  
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = offset;
// foreground layer  
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A4;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0x8000ffff;
  hdma2d.LayerCfg[1].InputOffset = 0;
// background layer  
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0xFF;
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
  uint32_t fg_addr = (uint32_t)tft_pen_img;
  uint32_t bg_addr = tft_addr + 2 * (50 * TFT_WIDTH + 50);
// uint32_t dst_addr = tft_addr + 2 * 500;
   
  if (HAL_DMA2D_BlendingStart(&hdma2d, fg_addr, bg_addr, bg_addr, 100, 100) != HAL_OK) {
    asm("nop");
    return;
  }  
  HAL_DMA2D_PollForTransfer(&hdma2d, 200);
*/
  
}  

void tftSwitchLayerAdressTest() {
  
  tstPrepareImg();
  
  tstInitBalls();

  tftLEDsetInst(200);
  
//  tftSelectFont("F16X32.FNT");
//  tftSetTextTransparency(0);
//  tftSelectBMP("BAT.BMP",0xffffffff);

//  tftSelectBMP("BAT.BMP",0x00ff00);
//    tftSelectBMP("BAT.BMP",0xffffffff);

  
  while (1) {
    tftDrawLayer0();

    tftSelectBMP("BAT.BMP",0xffffffff);
    tftDrawBMP(400,50);  
    
    tftSelectBMP("BAT.BMP",0x00ff00);
    tftDrawBMP(300,100);  

    tftSetForeground(0xffffff);
    tftSetPenWidth(1);
    tftSetPenPattern(0xF0F0F0F0);
    tftLine(0, 0, 799, 479);
    tftSetPenWidth(2);
    tftLine(0, 478, 798, 0);
    tftSetPenWidth(3);
    tftLine(10, 50, 700, 400);
    tftSetPenWidth(4);
    tftLine(10, 100, 700, 450);
    tftSetPenWidth(6);
    tftSetPenPattern(0x33333333);
    tftLine(10, 450, 750, 20);

    tftSetPenWidth(21);
    tftSetPenPattern(0xFFFFFFFF);
    tftSetForeground(0x0000ff);
    tftLine(40, 200, 120, 240);
    tftSetPenWidth(31);
    tftSetForeground(0x00ff00);
    tftLine(120, 240, 200, 210);
    tftSetPenWidth(24);
    tftSetForeground(0x0000ff);
    tftLine(200, 210, 290, 260);
    tftSetPenWidth(16);
    tftSetForeground(0xff0000);
    tftLine(290, 260, 380, 310);
    
    tftTestDMA2D_A4();

    tftNextFrame();
    
    tftWaitForReload();
  }
  
}  


