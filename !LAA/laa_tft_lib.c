/******************************************************************************
 *  Basic library of graphical primitives 
 *   - supports DMA2D (background/parallel video memory operations)
 ******************************************************************************/
#include "laa_tft_lib.h"
#include "string.h"
#include "laa_global_utils.h"
#include "laa_sdcard.h"
#include "laa_tft_cache.h"
#include "laa_sdram.h"

extern DMA2D_HandleTypeDef hdma2d;
extern uint32_t tft_addr;      // Address of memory region to draw primitives

#define TFT_PS    2

typedef struct {
  uint8_t   w;
  uint8_t   h;
  uint16_t  bpc;   // bytes per char
  uint8_t   *d;    // pointer to pixels data
  char      name[13];  // 12char name + "*" for system fonts
} TFTfont;

uint8_t   tft_wait_dma = 1;
uint32_t  tft_fg = 0xffffff;
uint32_t  tft_bg = 0x000000;
TFTfont   tft_fnt;

/* wait_dma setter*/
void tftSetWaitDMA(uint8_t mode) {
  tft_wait_dma = mode;
}  

/* fg setter*/
void tftSetForeground(uint32_t color) {
  tft_fg = color;
}  

/* bg setter*/
void tftSetBackground(uint32_t color) {
  tft_bg = color;
}  

void tftClearScreen(uint32_t color) {
  uint32_t save_fg = tft_fg;
  tft_fg = color;
  tftRect(0, 0, TFT_W, TFT_H);
  tft_fg = save_fg;
}  

void tftRect(int16_t x, int16_t y, uint16_t w, uint16_t h) {
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
    if (HAL_DMA2D_Start(&hdma2d, tft_fg, addr, w, h) == HAL_OK) {
      if (tft_wait_dma) HAL_DMA2D_PollForTransfer(&hdma2d, 200);
    }  
  }  
}

const uint8_t fnt_header[] = {0x5F, 0x46, 0x6E, 0x74, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x01};

uint8_t *tftLoadFont(const char* name) {
  if (name[0] == '*') name++;
  if (!sdOpenForRead(name)) return 0;
  uint8_t *font = 0;
  do {
    uint8_t buf[12];
    uint8_t sdRead(uint8_t *buffer, uint32_t size);
    if (!sdRead(buf, 12)) break;
    if (memcmp(buf, fnt_header, 10)) break;
    uint8_t h = buf[10];
    uint8_t w = buf[11];
    if (h < 5)   break;
    if (h > 100) break;
    if (w < 5)   break;
    if (w > 100) break;
    uint16_t bpc = ((w + 7) / 8) * h;
    font = tftLocateCache((bpc << 8) + 8, name);
    font[16] = w;
    font[17] = h;
    sdRead(font + 20, bpc << 8);
  } while (0);
  sdClose();
  return font;
}  

/* Set font as active. Input paramemter: font name
 *  routine searches for font in the heap and assigns it to active vars
 *  if not found reads font from SD and allocates it in the heap
 *  if not found on SD assigns default font
 */
void tftSetFont(const char* name) {
  if (!strncmp(tft_fnt.name, name, 13)) return;
  uint8_t *found = tftFindObject(name);
  if (!found) {
    found = tftLoadFont(name);
  } 
  if (!found) {
    found = (uint8_t *)TFT_ROM_CACHE;
    tft_fnt.name[0] = 0;
  } else {
    strncpy(tft_fnt.name, name, 13);
  }  
  found += 16;
  tft_fnt.w = found[0];
  tft_fnt.h = found[1];
  tft_fnt.bpc = ((tft_fnt.w + 7) / 8) * tft_fnt.h;
  tft_fnt.d = found + 4;
}  


#if 0
#endif
