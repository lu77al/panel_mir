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
  uint8_t   *d;    // pointer to pixels data
  uint16_t  bpc;   // bytes per char
  uint8_t   w;
  uint8_t   h;
  int16_t   x;  // Current carret position
  int16_t   y;
  uint8_t   transparent;
  char      name[13];  // 12char name + "*" for system fonts
} TFTfont;

uint8_t   tft_wait_dma = 1;
uint32_t  tft_fg = 0xffffff;
uint32_t  tft_bg = 0x000000;
uint16_t  tft_fg16 = 0xffff;
uint16_t  tft_bg16 = 0x0000;

TFTfont   tft_fnt;

/* Color 24bit -> 16bit(565) */
uint16_t tftShrinkColor(uint32_t color) {
  return ((color & 0x0000F8) >> 3) | ((color & 0x00FC00) >> 5) | ((color & 0xF80000) >> 8);
}  

/* wait_dma setter*/
void tftSetWaitDMA(uint8_t mode) {
  tft_wait_dma = mode;
}  

/* fg setter*/
void tftSetForeground(uint32_t color) {
  tft_fg = color;
  tft_fg16 = tftShrinkColor(color);
}  

/* bg setter*/
void tftSetBackground(uint32_t color) {
  tft_bg = color;
  tft_bg16 = tftShrinkColor(color);
}  

/* text position setter */
void tftSetTextPos(int16_t x, int16_t y) {
  tft_fnt.x = x;
  tft_fnt.y = y;
}

/* text transparency setter */
void tftSetTextTransparency(int8_t tr) {
  tft_fnt.transparent = tr;
}  

/* Fill all active layer with colour */
void tftClearScreen(uint32_t color) {
  uint32_t save_fg = tft_fg;
  tft_fg = color;
  tftRect(0, 0, TFT_W, TFT_H);
  tft_fg = save_fg;
}  

/* Rectangle at x,y of w,h size with fg color (withput out;ine) */
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

/* Full char with background */
void tftCharFullBG(uint16_t *scr, uint8_t *chr) {
  for (uint8_t y = tft_fnt.h; y; y--) {
    uint16_t chunk = 0;
    uint16_t *pixel = scr;
    for (uint8_t x = tft_fnt.w; x; x--) {
      if ((uint8_t)chunk == 0) chunk = (*(chr++) << 8) | 0xff;
      *(pixel++) = (chunk & 0x8000) ? tft_fg16 : tft_bg16;
      chunk <<= 1;
    }
    scr += TFT_W;
  }
}  

/* Full char transparent */
void tftCharFullTR(uint16_t *scr, uint8_t *chr) {  
  for (uint8_t y = tft_fnt.h; y; y--) {
    uint16_t chunk = 0;
    uint16_t *pixel = scr;
    for (uint8_t x = tft_fnt.w; x; x--) {
      if ((uint8_t)chunk == 0) chunk = (*(chr++) << 8) | 0xff;
      if (chunk & 0x8000) *pixel = tft_fg16;
      pixel++;
      chunk <<= 1;
    }
    scr += TFT_W;
  }
}  

/* Clipped character (at the edge of screen */
void tftCharClipped(uint8_t *chr) { 
  int16_t x_lim = tft_fnt.x + tft_fnt.w;
  int16_t y_lim = tft_fnt.y + tft_fnt.h;
  uint16_t *line_addr = (uint16_t *)tft_addr + tft_fnt.y * TFT_W;
  for (int16_t y = tft_fnt.y; y < y_lim; y++) {
    uint16_t chunk = 0;
    for (int16_t x = tft_fnt.x; x < x_lim; x++) {
      if ((uint8_t)chunk == 0) chunk = (*(chr++) << 8) | 0xff;
      do {
        if (x < 0) break;
        if (y < 0) break;
        if (x >= TFT_W) break;
        if (y >= TFT_H) break;
        if (chunk & 0x8000) {
          *(line_addr + x) = tft_fg16;
        } else if (!tft_fnt.transparent) {
          *(line_addr + x) = tft_bg16;
        }  
      } while (0);
      chunk <<= 1;
    }
    line_addr += TFT_W;
  }  
}  

/* Draw one character and move carret pos */
void tftChar(char ch) {
  if (tft_fnt.d == 0) return;
  if (tft_fnt.x >= TFT_W) return;
  if (tft_fnt.y >= TFT_H) return;
  if ((tft_fnt.x + tft_fnt.w) < 1) return;
  if ((tft_fnt.y + tft_fnt.h) < 1) return;
  uint8_t *chr = tft_fnt.d + ch * tft_fnt.bpc;
  if ((tft_fnt.x < 0) || (tft_fnt.y < 0) ||
      ((tft_fnt.x + tft_fnt.w) > TFT_W) ||
      ((tft_fnt.y + tft_fnt.h) > TFT_H)) {
    tftCharClipped(chr);
  } else {
    uint16_t *scr = (uint16_t *)tft_addr + tft_fnt.y * TFT_W + tft_fnt.x;
    if (tft_fnt.transparent) {
      tftCharFullTR(scr, chr);
    } else {
      tftCharFullBG(scr, chr);
    }  
  }
  tft_fnt.x += tft_fnt.w;
}  

/* Print string */
void tftPrint(char *text, uint8_t length) {
  for (uint8_t i = length; i; i--) {
    if (!*text) return;
    tftChar(*(text++));
  }  
}  



/*
void tftDrawChar(char ch) {
  if (tft_fnt.d == 0) return;
  if (tft_fnt.x >= TFT_W) return;
  if (tft_fnt.y >= TFT_H) return;
  if ((tft_fnt.x + tft_fnt.w) < 1) return;
  if ((tft_fnt.y + tft_fnt.h) < 1) return;
  
  if (((fnt_x + fnt_w) > TFT_W) || ((fnt_y + fnt_h) > TFT_H) || (fnt_x < 0) || (fnt_y < 0)) {
    uint8_t  *fm = fnt_addr + ch * fnt_bpc;
    int16_t xlim = fnt_x + fnt_w;
    int16_t ylim = fnt_y + fnt_h;
    for (int16_t y = fnt_y; y < ylim; y++) {
      uint16_t line = 0;
      for (int16_t x = fnt_x; x < xlim; x++) {
        if (!(line & 0xff)) line = (*(fm++) << 8) | 0xff;
        while (1) {
          if (x < 0) break;
          if (y < 0) break;
          if (x >= TFT_W) break;
          if (y >= TFT_H) break;
          if (line & 0x8000) {
            *((uint16_t *)tft_addr + (x + (y * TFT_W))) = fg_color;
          } else if (!fnt_transparent) {
            *((uint16_t *)tft_addr + (x + (y * TFT_W))) = bg_color;
          }  
          break;
        }  
        line <<= 1;
      }  
    }  
  } else {
    uint16_t *vm = (uint16_t *)tft_addr + (fnt_x + fnt_y * TFT_W);
    uint8_t  *fm = fnt_addr + ch * fnt_bpc;
    if (fnt_transparent) {
      for (uint8_t y = fnt_h; y; y--) {
        uint16_t line = 0;
        uint16_t *pix = vm;
        for (uint8_t x = fnt_w; x; x--) {
          if (!(line & 0xff)) line = (*(fm++) << 8) | 0xff;
          if (line & 0x8000) *pix = fg_color;
          pix++;
          line <<= 1;
        }
        vm += TFT_W;
      }
    } else {
      for (uint8_t y = fnt_h; y; y--) {
        uint16_t line = 0;
        uint16_t *pix = vm;
        for (uint8_t x = fnt_w; x; x--) {
          if (!(line & 0xff)) line = (*(fm++) << 8) | 0xff;
          *(pix++) = (line & 0x8000) ? fg_color : bg_color;
          line <<= 1;
        }
        vm += TFT_W;
      }
    }  
  }  
}
*/

/* *.fnt file normal header (see reqspec) */
const uint8_t fnt_header[] = {0x5F, 0x46, 0x6E, 0x74, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x01};

/* Load font from SD card to cache (locates space inside routine)*/
uint8_t *tftLoadFont(const char* name) {
  if (name[0] == '*') name++; // Try to read system font from SD card
  if (!sdOpenForRead(name)) return 0;
  uint8_t *font_obj = 0;
  do {
    uint8_t buf[12];
    if (!sdRead(buf, 12)) break;
    if (memcmp(buf, fnt_header, 10)) break;
    uint8_t h = buf[10];
    uint8_t w = buf[11];
    if (h < 5)   break;
    if (h > 100) break;
    if (w < 5)   break;
    if (w > 100) break;
    uint16_t bpc = ((w + 7) / 8) * h; // bytes per char (line consistst of integer amount of bytes)
    font_obj = tftLocateCache((bpc << 8) + 4, name);
    font_obj[16] = w;
    font_obj[17] = h;
    *(uint16_t *)(&font_obj[18]) = bpc;
    sdRead(font_obj + 20, bpc << 8);
  } while (0);
  sdClose();
  return font_obj;
}  

/* Set font as active. Input paramemter: font name
 *  routine searches for font in the heap and assigns it to active vars
 *  if not found reads font from SD and allocates it in the heap
 *  if not found on SD assigns default font
 */
void tftSetFont(const char* name) {
  if (!strncmp(tft_fnt.name, name, 13) && tft_fnt.d) return;
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
  tft_fnt.w = found[16];
  tft_fnt.h = found[17];
  tft_fnt.bpc = *(uint16_t *)(&found[18]);
  tft_fnt.d = found + 20;
}  


/* Clear cache and assingn current font and image to null
 */
void tftResetObjects() {
  tftClearCache();
  tft_fnt.d = 0;
  tft_fnt.x = 0;
  tft_fnt.y = 0;
  tft_fnt.transparent = 1;
}

#if 0
#endif
