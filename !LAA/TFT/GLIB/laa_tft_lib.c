/******************************************************************************
 *  Basic library of graphical primitives 
 *   - supports DMA2D (background/parallel video memory operations)
 ******************************************************************************/
#include "laa_tft_lib.h"
#include "string.h"
#include "laa_global_utils.h"
#include "laa_sdcard.h"
#include "laa_tft_cache.h"
#include "laa_tft_led.h"
#include "laa_sdram.h"

extern DMA2D_HandleTypeDef hdma2d;

uint32_t tft_addr  = TFT_SCREEN;  // Address of memory region to draw primitives
uint8_t  tft_wait_dma = 1;

uint32_t  tft_fg = 0xffffff;
uint32_t  tft_bg = 0x000000;
uint16_t  tft_fg16 = 0xffff;
uint16_t  tft_bg16 = 0x0000;

typedef struct {
  uint8_t   *img;  // pointer to pixels data
  uint16_t  bpc;   // bytes per char
  uint8_t   w;
  uint8_t   h;
  int16_t   x;  // Current carret position
  int16_t   y;
  uint8_t   transparent;
  char      name[13];  // 12char name + "*" for system fonts
} TFTfont;

TFTfont  tft_fnt;

typedef struct {
  uint8_t   *img;      // pointer to pixels data
  uint16_t  w;
  uint16_t  h;
  uint16_t  trColor;   // transparent color (16 bit) 0 - not set
  char      name[17];  // 12char name + "*" for system fonts + transparent_color_hash
} TFTbmp;

TFTbmp  tft_bmp;

typedef struct {
  int16_t   x;
  int16_t   y;
  uint8_t   width;
  uint32_t  pattern;     
  uint32_t  patternPoint;
  uint16_t  pathCounter;
  uint8_t   bigWidth;
  int32_t   ptrUL[130];
  int32_t   ptrUR[130];
  int32_t   ptrDL[130];
  int32_t   ptrDR[130];
  uint8_t   sLen[130];
  uint8_t   sCnt;
} TFTpen;

TFTpen  tft_pen;

void tftDrawLine(int16_t x1, int16_t y1, uint16_t x2, uint16_t y2);

//*********** COLORS + GENERAL ROUTINES **************

/* Clear cache and assign current font and image to null
 */
void tftResetObjects() {
  tftClearCache();
  tft_fnt.img = 0;
  tft_fnt.x = 0;
  tft_fnt.y = 0;
  tft_fnt.transparent = 1;
//  tft_pen.pattern = 0xffffffff;
  tftSetPenWidth(1);
  tft_pen.bigWidth = 0;
  tftSetPenPattern(0x0f0f0f0f);
  tft_pen.patternPoint = 1;
}

/* Fill all active layer with colour */
void tftClearScreen(uint32_t color) {
  uint32_t save_bg = tft_bg;
  tft_bg = color;
  tftRect(0, 0, TFT_WIDTH, TFT_HEIGHT);
  tft_bg = save_bg;
}  

/* Color 24bit -> 16bit(565) */
uint16_t tftShrinkColor565(uint32_t color) {
  return ((color & 0x0000F8) >> 3) | ((color & 0x00FC00) >> 5) | ((color & 0xF80000) >> 8);
}  

/* Color 24bit -> 16bit(555) */
uint16_t tftShrinkColor555(uint32_t color) {
  return (0x8000 | (color & 0x0000F8) >> 3) | ((color & 0x00F800) >> 6) | ((color & 0xF80000) >> 9);
}  

/* wait_dma setter*/
void tftSetWaitDMA(uint8_t mode) {
  tft_wait_dma = mode;
}  

/* fg setter*/
void tftSetForeground(uint32_t color) {
  tft_fg = color;
  tft_fg16 = tftShrinkColor565(color);
}  

/* bg setter*/
void tftSetBackground(uint32_t color) {
  tft_bg = color;
  tft_bg16 = tftShrinkColor565(color);
}  

void tftSetPenWidth(uint8_t w) {
  tft_pen.width = w;
  if (w > 6) {
    if (tft_pen.bigWidth == w) return;
    tft_pen.bigWidth = w;
    uint16_t full_sq = w * w;
    uint8_t  inner_hord = (w & 1) + 1;
    uint8_t  inner_diam = w - 1;
    uint8_t  radius = (w + 1) >> 1;
    uint8_t  bakHord = 0;
    int32_t  top = - (w >> 1) * (TFT_WIDTH + 1);
    int32_t  bot = top + TFT_WIDTH * (w - 1);
    for (uint8_t i = 0; i < radius; i++) {
      uint16_t val = full_sq -  inner_diam * inner_diam;
      while (inner_hord * inner_hord <= val) inner_hord +=2;
      uint8_t hord = inner_hord - 1;
      if (hord > w) hord = w;
      tft_pen.sLen[i] = ((hord - bakHord) >> 1) + 1;
      tft_pen.ptrUL[i] = top + ((w - hord) >> 1);
      tft_pen.ptrDL[i] = bot + ((w - hord) >> 1);
      tft_pen.ptrUR[i] = tft_pen.ptrUL[i] + hord - tft_pen.sLen[i];
      tft_pen.ptrDR[i] = tft_pen.ptrDL[i] + hord - tft_pen.sLen[i];
      bakHord = hord;
      top += TFT_WIDTH;
      bot -= TFT_WIDTH;
      inner_diam -= 2;
    }
    tft_pen.sCnt = radius;
  }  
}  

void tftSetPenPattern(uint32_t pattern) {
  tft_pen.pattern = pattern;
}  

//*********** BASIC PRIMITIVES **************

/* Rectangle at x,y of w,h size with bg color (withput outline) */
void tftRect(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  if ((w | h) == 0) return;
  if (x >= TFT_WIDTH) return;
  if (y >= TFT_HEIGHT) return;
  if ((x + w) < 1) return;
  if ((y + h) < 1) return;
  if ((x + w) > TFT_WIDTH) w = TFT_WIDTH - x;
  if ((y + h) > TFT_HEIGHT) h = TFT_HEIGHT - y;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  uint32_t addr = tft_addr + TFT_PIXEL * (y * TFT_WIDTH + x);
  hdma2d.Init.Mode = DMA2D_R2M;
  hdma2d.Init.OutputOffset = TFT_WIDTH - w;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) return;
  if (HAL_DMA2D_Start(&hdma2d, tft_bg, addr, w, h) != HAL_OK) return;
  if (tft_wait_dma) HAL_DMA2D_PollForTransfer(&hdma2d, 200);
}

void tftMoveTo(int16_t x, int16_t y) {
  tft_pen.patternPoint = 1;
  tft_pen.pathCounter = 0;
  tft_pen.x = x;
  tft_pen.y = y;
}  

void tftLineTo(int16_t x, int16_t y) {
  tftDrawLine(tft_pen.x, tft_pen.y, x, y);
}  

void tftLineRel(int16_t x, int16_t y) {
  tftDrawLine(tft_pen.x, tft_pen.y, tft_pen.x + x, tft_pen.y + y);
}  

void tftLine(int16_t x1, int16_t y1, uint16_t x2, uint16_t y2) {
  tft_pen.patternPoint = 1;
  tft_pen.pathCounter = 0;
  tftDrawLine(x1, y1, x2, y2);
}  

void tftDrawLinePointSeg(uint16_t *center, int32_t *ptr) {
  uint8_t *len = tft_pen.sLen;
  uint16_t c = tft_fg16;
  for (uint8_t i = tft_pen.sCnt; i; i--) {
    uint16_t *p = center + *ptr;
    for (uint8_t j = *len; j; j--)*(p++) = c;
    ptr++;
    len++;
  }  
}  

void tftDrawLinePoint(uint16_t *center) {
  uint8_t *len = tft_pen.sLen;
  int32_t *UL = tft_pen.ptrUL;
  int32_t *UR = tft_pen.ptrUR;
  int32_t *DL = tft_pen.ptrDL;
  uint16_t c = tft_fg16;
  uint16_t *p;
  uint8_t hord; 
  for (uint8_t i = tft_pen.sCnt; i; i--) {
    hord = *UR - *UL + *len;
    p = center + *UL;
    for (uint8_t j = hord; j; j--)*(p++) = c;
    p = center + *DL;
    for (uint8_t j = hord; j; j--)*(p++) = c;
    len++;
    UL++;
    UR++;
    DL++;
  }  
}  

void tftDrawLine(int16_t x1, int16_t y1, uint16_t x2, uint16_t y2) {
// Update pen position, check pathQuantum
  tft_pen.x = x2;
  tft_pen.y = y2;
// Validate parameters  **
  if (!tft_pen.width) return;
  uint8_t right_margin = (tft_pen.width - 1) >> 1;
  uint8_t left_margin = tft_pen.width - right_margin;
  if (x1 < right_margin) return;
  if (y1 < right_margin) return;
  if (x2 < right_margin) return;
  if (y2 < right_margin) return;
  if (x1 > TFT_WIDTH - left_margin) return;
  if (y1 > TFT_HEIGHT - left_margin) return;
  if (x2 > TFT_WIDTH - left_margin) return; 
  if (y2 > TFT_HEIGHT - left_margin) return;
// Prepare additional data
  int16_t    mainLen, slaveLen;
  int32_t    mainStep, slaveStep;
  if (x1 <= x2) {
    mainLen = x2 - x1;
    mainStep = 1;
  } else {
    mainLen = x1 - x2;
    mainStep = -1;
  }
  if (y1 <= y2) {
    slaveLen = y2 - y1;
    slaveStep = TFT_WIDTH;
  } else {
    slaveLen = y1 - y2;
    slaveStep = -TFT_WIDTH;
  }
  if (mainLen < slaveLen) {
    SWAP16(mainLen,  slaveLen);
    SWAP32(mainStep, slaveStep);
  }
  uint16_t counter = mainLen >> 1;
  uint16_t *point = (uint16_t *)tft_addr + x1 + y1 * TFT_WIDTH;
  uint16_t *bakPoint = point;
  uint32_t pattern = tft_pen.pattern;
  uint32_t patternPoint = tft_pen.patternPoint;
  uint16_t pathCounter = tft_pen.pathCounter;
  uint16_t pathQuantum = 176 * tft_pen.width;
  uint16_t color = tft_fg16;
  uint8_t  width = tft_pen.width;
// Draw it
  for (uint16_t i = mainLen + 1; i; i--) {
    if (pattern & patternPoint) {
      if (width == 1) { // Width = 1
        *point = color;
      } else {
        uint16_t *p;
        uint16_t c = color;        
        if (width == 2) { // Width = 2
          p = point;
          *(p++)=c; *p=c;          //  0*
          p += TFT_WIDTH - 1;      //  **
          *(p++)=c; *p=c;
        } else if (width == 3) { // Width = 3
          p = point - TFT_WIDTH;
          *p=c; p += TFT_WIDTH - 1;  //  *
          *(p++)=c; *(p++)=c; *p=c;  // *0*
          p += TFT_WIDTH - 1; *p=c;  //  *
        } else if (width == 4) { // Width = 4
          p = point - TFT_WIDTH;
          *(p++)=c; *p=c; p += TFT_WIDTH - 2;  //  **
          *(p++)=c; *(p++)=c; *(p++)=c; *p=c;  // *0**
          p += TFT_WIDTH - 3;                  // **** 
          *(p++)=c; *(p++)=c; *(p++)=c; *p=c;  //  **
          p += TFT_WIDTH - 2;
          *(p++)=c; *p=c;
        } else if (width == 5) { // Width = 5
          p = point - TFT_WIDTH * 2 - 1;
          *(p++)=c; *(p++)=c; *p=c;
          p += TFT_WIDTH - 3;
          *(p++)=c; *(p++)=c; *(p++)=c; *(p++)=c; *p=c;      //  ***
          p += TFT_WIDTH - 4;                                // *****
          *(p++)=c; *(p++)=c; *(p++)=c; *(p++)=c; *p=c;      // **0**
          p += TFT_WIDTH - 4;                                // *****
          *(p++)=c; *(p++)=c; *(p++)=c; *(p++)=c; *p=c;      //  ***
          p += TFT_WIDTH - 3;
          *(p++)=c; *(p++)=c; *p=c;
        } else if (width == 6) { // Width = 6
          p = point - TFT_WIDTH * 2;
          *(p++)=c; *p=c;
          p += TFT_WIDTH - 2;
          *(p++)=c; *(p++)=c; *(p++)=c; *p=c;                      //   **
          p += TFT_WIDTH - 4;                                      //  ****
          *(p++)=c; *(p++)=c; *(p++)=c; *(p++)=c; *(p++)=c; *p=c;  // **0***
          p += TFT_WIDTH - 5;                                      // ******
          *(p++)=c; *(p++)=c; *(p++)=c; *(p++)=c; *(p++)=c; *p=c;  //  ****      
          p += TFT_WIDTH - 4;                                      //   **
          *(p++)=c; *(p++)=c; *(p++)=c; *p=c;
          p += TFT_WIDTH - 2;
          *(p++)=c; *p=c;
        } else {
          int32_t step = point - bakPoint;
          if (step == 0) tftDrawLinePoint(point); else {
            uint8_t up = (step < -2);
            uint8_t down = (step > 2);
            if (down) step -= TFT_WIDTH;
            if (up) step += TFT_WIDTH;
            uint8_t right = (step == 1);
            uint8_t left = (step == -1);
            if (up || right)   tftDrawLinePointSeg(point, tft_pen.ptrUR);
            if (up || left)    tftDrawLinePointSeg(point, tft_pen.ptrUL);
            if (down || right) tftDrawLinePointSeg(point, tft_pen.ptrDR);
            if (down || left)  tftDrawLinePointSeg(point, tft_pen.ptrDL);
          }  
          bakPoint = point;
        }  
      }  
    }  
    point += mainStep;
    if ((counter += slaveLen) >= mainLen) {
      counter -= mainLen;
      point += slaveStep;
      pathCounter += 73;
    }
    pathCounter += 176;
    while (pathCounter >= pathQuantum) {
      pathCounter -= pathQuantum;
      patternPoint <<= 1;
      if (patternPoint == 0) patternPoint = 1;
    }  
  }  
  tft_pen.patternPoint = patternPoint;
  tft_pen.pathCounter = pathCounter;
}  

typedef struct {
  int16_t x,y;
} TPoint;

typedef struct { // Task oriented strucure for drawing polygon
  int16_t  top;        // top..bot - work range
  int16_t  bot;
  int16_t  scan;       // current scan line
  TPoint   vtx[254];   // vertexes
  uint8_t  size;       // vertexes count
  uint8_t  stage;      // stage of draw (prepare, fill, outline) 
} TPolyTask;

TPolyTask  tft_poly;

void tftPolyInit(uint8_t filled) {
  tft_poly.stage = filled ? 0 : 2;
  tft_poly.size = 0;
}

void tftPolyAddVertex(int16_t x, int16_t y) {
  TPolyTask *p = &tft_poly;
  if (p->size == 255) return;
  p->vtx[p->size].x = x;
  p->vtx[p->size].y = y;
  p->size++;
}

void tftPolyScanSection(int16_t y, int16_t x1, int16_t x2) {
  if (x1 > x2) SWAP16(x1, x2);
  if (x2 < 0) return;
  if (x1 >= TFT_WIDTH) return;
  if (x1 < 0) x1 = 0;
  if (x2 >= TFT_WIDTH) x2 = TFT_WIDTH - 1;
  HAL_DMA2D_Start(&hdma2d, tft_bg,
                  tft_addr + TFT_PIXEL * (x1 + y * TFT_WIDTH),
                  x2 - x1 + 1, 1);
  HAL_DMA2D_PollForTransfer(&hdma2d, 50);
}  

void tftPolyDrawScanLine() {
  int16_t x[255];
  uint8_t xCnt = 0;
  int16_t y = tft_poly.scan;
  TPoint *pLast = tft_poly.vtx + tft_poly.size - 1;
  TPoint *p1;
  TPoint *p2 = tft_poly.vtx;
  while (p2->y == y) p2++;
  uint8_t up = p2->y < y;
  for (uint8_t i=tft_poly.size; i; i--) {
    p1 = p2;
    p2 = p1 != pLast ? p1 + 1 : tft_poly.vtx;
    if (p2->y != y) {
      uint8_t newUp = p2->y < y;
      if (up != newUp) {
        up = newUp;
        int32_t tmp  = (p2->x - p1->x) * (y - p1->y);
        int16_t xNew = p1->x + tmp / (p2->y - p1->y);
        uint8_t ins = 0;
        while ((xNew > x[ins]) && (ins < xCnt)) ins++;
        memmove(&x[ins+1], &x[ins], (xCnt - ins)*2);
        x[ins] = xNew;
        xCnt++;
      }  
    }
  }
  for (uint8_t i=0; i < xCnt>>1; i++) tftPolyScanSection(y, x[i*2], x[i*2+1]);
}  

uint8_t tftPloyProcess() {
  TPolyTask *p = &tft_poly;
  if (p->size < 3) return 0;
  if (p->stage == 0) { // Init params (top/bot, draw horizontals, vertexes, DMA2D init)
    p->bot = 0;
    p->top = TFT_HEIGHT - 1;
    hdma2d.Init.Mode = DMA2D_R2M;
    HAL_DMA2D_Init(&hdma2d);
    TPoint *p1;
    TPoint *p2 = p->vtx;
    for (int8_t i=p->size; i; i--) {
      p1 = p2;
      p2 = i > 1 ? p1 + 1 : p->vtx;
      if (p->top > p1->y) p->top = p1->y;
      if (p->bot < p1->y) p->bot = p1->y;
      if (p1->y < 0) continue;
      if (p1->y >= TFT_HEIGHT) continue;
      if (p1->y == p2->y) {
        tftPolyScanSection(p1->y, p1->x, p2->x);
      } else {
        if (p1->x < 0) continue;
        if (p1->x >= TFT_WIDTH) continue;
        *((uint16_t *)tft_addr + p1->x + p1->y * TFT_WIDTH) = tft_bg16;
      }  
    }
    p->scan = p->top;
    if (p->top < 0) p->top = 0;
    if (p->bot >= TFT_HEIGHT) p->bot = TFT_HEIGHT - 1;
    p->stage++;
  } else if (p->stage == 1) {
    for (uint8_t i=32; i; i--) {
      p->scan++;
      if (p->scan >= p->bot) {
        p->stage++;
        break;
      }  
      tftPolyDrawScanLine();
    }  
  } else if (p->stage == 2) {
    if (tft_pen.width) {
      TPoint *pnt = &p->vtx[p->size-1];
      tftMoveTo(pnt->x, pnt->y);
      pnt = p->vtx;
      for (uint8_t i=p->size; i; i--) {
        tftLineTo(pnt->x, pnt->y);
        pnt++;
      }  
    }
    p->size = 0;
  }
  return p->size >= 3;
}

//*********** FONT AND TEXT ROUTINES **************

/* text position setter */
void tftSetTextPos(int16_t x, int16_t y) {
  tft_fnt.x = x;
  tft_fnt.y = y;
}

/* text transparency setter */
void tftSetTextTransparency(int8_t tr) {
  tft_fnt.transparent = tr;
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
    scr += TFT_WIDTH;
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
    scr += TFT_WIDTH;
  }
}  

/* Clipped character (at the edge of screen */
void tftCharClipped(uint8_t *chr) { 
  int16_t x_lim = tft_fnt.x + tft_fnt.w;
  int16_t y_lim = tft_fnt.y + tft_fnt.h;
  uint16_t *line_addr = (uint16_t *)tft_addr + tft_fnt.y * TFT_WIDTH;
  for (int16_t y = tft_fnt.y; y < y_lim; y++) {
    uint16_t chunk = 0;
    for (int16_t x = tft_fnt.x; x < x_lim; x++) {
      if ((uint8_t)chunk == 0) chunk = (*(chr++) << 8) | 0xff;
      do {
        if (x < 0) break;
        if (y < 0) break;
        if (x >= TFT_WIDTH) break;
        if (y >= TFT_HEIGHT) break;
        if (chunk & 0x8000) {
          *(line_addr + x) = tft_fg16;
        } else if (!tft_fnt.transparent) {
          *(line_addr + x) = tft_bg16;
        }  
      } while (0);
      chunk <<= 1;
    }
    line_addr += TFT_WIDTH;
  }  
}  

/* Draw one character and move carret pos */
void tftDrawChar(char ch) {
  if (tft_fnt.img == 0) return;
  if (tft_fnt.x >= TFT_WIDTH) return;
  if (tft_fnt.y >= TFT_HEIGHT) return;
  if ((tft_fnt.x + tft_fnt.w) < 1) return;
  if ((tft_fnt.y + tft_fnt.h) < 1) return;
  uint8_t *chr = tft_fnt.img + ch * tft_fnt.bpc;
  if ((tft_fnt.x < 0) || (tft_fnt.y < 0) ||
      ((tft_fnt.x + tft_fnt.w) > TFT_WIDTH) ||
      ((tft_fnt.y + tft_fnt.h) > TFT_HEIGHT)) {
    tftCharClipped(chr);
  } else {
    uint16_t *scr = (uint16_t *)tft_addr + tft_fnt.y * TFT_WIDTH + tft_fnt.x;
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
    tftDrawChar(*(text++));
  }  
}  

/* *.fnt file normal header (see reqspec) */
const uint8_t fnt_header[] = {0x5F, 0x46, 0x6E, 0x74, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x01};

/* Load font from SD card to cache (locates space inside routine)*/
uint8_t *tftLoadFont(const char* name) {
  if (!sdOpenForRead(name)) return 0;
  if (name[0] == '*') name++; // Try to read system font from SD card
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
    font_obj[0] = w;
    font_obj[1] = h;
    *(uint16_t *)(&font_obj[2]) = bpc;
    sdRead(font_obj + 4, bpc << 8);
  } while (0);
  sdClose();
  return font_obj;
}  

/* Set font as active. Input paramemter: font name
 *  routine searches for font in the heap and assigns it to active vars
 *  if not found reads font from SD and allocates it in the heap
 *  if not found on SD assigns default font
 */
void tftSelectFont(const char* name) {
  if (!strncmp(tft_fnt.name, name, 13) && tft_fnt.img) return;
  uint8_t *found = tftFindObject(name);
  if (!found) {
    found = tftLoadFont(name);
    if (found) strncpy(tft_fnt.name, name, 13);
  } 
  if (!found) {
    found = (uint8_t *)TFT_ROM_CACHE;
    tft_fnt.name[0] = 0;
  }  
  tft_fnt.w = found[0];
  tft_fnt.h = found[1];
  tft_fnt.bpc = MEM16(found + 2);
  tft_fnt.img = found + 4;
}  

//*********** BITMAPS **************
  #define IMG_PIXEL 2

/* Copy BPM image to screen buffer with transparent color
 */
void tftCopyTransparentBMP(uint32_t dst, uint32_t src, int16_t w, int16_t h) {
  int16_t dst_offset = TFT_WIDTH - w;
  int16_t src_offset = tft_bmp.w - w;
// output  
  hdma2d.Init.Mode = DMA2D_M2M_BLEND;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = dst_offset;
// foreground layer  
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB1555;
  hdma2d.LayerCfg[1].InputOffset = src_offset;
// background layer  
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0xFF;
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[0].InputOffset = dst_offset;
// Apply configuration
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) return;
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 0) != HAL_OK) return;
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) return;
// start transfer  
  if (HAL_DMA2D_BlendingStart(&hdma2d, src, dst, dst, w, h) != HAL_OK) return;
}  

/* Copy BPM image to screen buffer as is
 */
void tftCopyOpaqueBMP(uint32_t dst, uint32_t src, int16_t w, int16_t h) {
  int16_t dst_offset = TFT_WIDTH - w;
  int16_t src_offset = tft_bmp.w - w;
// output  
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = dst_offset;
// foreground layer  
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].InputOffset = src_offset;
// Apply configuration
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) return;
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) return;
// start transfer  
  if (HAL_DMA2D_Start(&hdma2d, src, dst, w, h) != HAL_OK) return;
}  

/* Copy BPM image to screen buffer as is
 *  - clip to screen, prepare adresses, start trasparent or opaque mode
 */
void tftDrawBMP(int16_t x, int16_t y) {
  if (tft_bmp.img == 0) return;
  if (x >= TFT_WIDTH) return;
  if (y >= TFT_HEIGHT) return;
  int16_t w = tft_bmp.w;
  int16_t h = tft_bmp.h;
  if ((x + w) < 1) return;
  if ((y + h) < 1) return;
  if ((x + w) > TFT_WIDTH) w = TFT_WIDTH - x;
  if ((y + h) > TFT_HEIGHT) h = TFT_HEIGHT - y;
  int16_t bmp_x = 0;
  int16_t bmp_y = 0;
  if (x < 0) { w += x; bmp_x = -x; x = 0; }
  if (y < 0) { h += y; bmp_y = -y; y = 0; }
  uint32_t dst_addr = tft_addr + TFT_PIXEL * (y * TFT_WIDTH + x);
  uint32_t src_addr = (uint32_t)tft_bmp.img + IMG_PIXEL * (bmp_y * tft_bmp.w + bmp_x);
  if (tft_bmp.trColor) {
    tftCopyTransparentBMP(dst_addr, src_addr, w, h);
  } else {
    tftCopyOpaqueBMP(dst_addr, src_addr, w, h);
  } 
  if (tft_wait_dma) HAL_DMA2D_PollForTransfer(&hdma2d, 200);
}  


/* Prepare DM2D transfer for reading BMP from SD
 */
uint8_t tftPrepareDMA2D_LOAD_BMP(uint8_t transparent) {
  hdma2d.Init.Mode = DMA2D_M2M_PFC;
  hdma2d.Init.ColorMode = transparent ? DMA2D_OUTPUT_ARGB1555 : DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d.LayerCfg[1].InputOffset = 0;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) return 0;
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) return 0;
  return 1;
}  

void tftAddTransparency(uint16_t *data, uint16_t length, uint16_t trColor) {
  for (;length; length--) {
    if (*data == trColor) *data = 0;
    data++;
  }  
}  

uint8_t *tftLoadBMP(const char* name, const char* extendedName, uint16_t trColor) {
  if (!sdOpenForRead(name)) return 0;
  if (!tftPrepareDMA2D_LOAD_BMP(trColor)) return 0;
  if (name[0] == '*') name++; // Try to read system bmp image from SD card
  uint8_t *bmp_obj = 0;
  do {
    uint8_t buf[26];
    if (!sdRead(buf, 26)) break; // Read header
    if (*(uint16_t *)buf != 0x4D42) break;
    uint32_t offset = MEM32(buf + 10); // image offset
    if (offset < 54) break;
    if (!sdSeek(offset)) break;
    uint32_t width = MEM32(buf + 18);  // width
    if (width == 0) break;
    if (width > TFT_WIDTH) break;
    uint32_t height = MEM32(buf + 22); // heigt
    if (height == 0) break;
    if (height > TFT_HEIGHT) break;
    bmp_obj = tftLocateCache(width * height * IMG_PIXEL + 4, extendedName); // locate cache memory
    SAVE_MEM16(bmp_obj, width)
    SAVE_MEM16(bmp_obj + 2, height)
    uint32_t src_line_size  = (width * 3 + 3) & 0xfffffffc;
    uint16_t *load_addr = (uint16_t *)(bmp_obj + 4 + width * (height - 1) * IMG_PIXEL);
    uint16_t *conv_addr = load_addr;
    uint16_t load_cnt = height;
    uint16_t conv_cnt = trColor ? height : 0;
    uint8_t  load_handicap = 4;
    HAL_DMA2D_PollForTransfer(&hdma2d, 100);
    while (load_cnt | conv_cnt) {
      if (load_cnt) {
        sdRead((void *)sd_buf, src_line_size);
        HAL_DMA2D_Start(&hdma2d, (uint32_t)sd_buf, (uint32_t)load_addr, width, 1);
        load_addr -= width;
        load_cnt--;
      }
      if (load_handicap) load_handicap--; else {
        if (!load_cnt) HAL_DMA2D_PollForTransfer(&hdma2d, 100);
        if (conv_cnt) {
          tftAddTransparency(conv_addr, width, trColor);
          conv_addr -= width;
          conv_cnt--;
        }  
      } 
      HAL_DMA2D_PollForTransfer(&hdma2d, 100);
    }  
  } while (0);
  sdClose();
  return bmp_obj;
}

/* Set BMP as active. Input paramemter: BMP name, transparemt color
 *  routine searches for BMP in the heap and assigns it to active vars
 *  if not found reads BMP from SD and allocates it in the heap
 *  if not found on SD assigns nul
 */
void tftSelectBMP(const char* name, uint32_t trColor888) {
  char extendedName[18]; // Prepare extended name
  // (system prefix * up to 1 byte) + (name.ext up to 12 bytes) + (transparent color 4 bytes)
  strncpy(extendedName, name, 13);
  uint16_t trColor = (trColor888 != 0xffffffff) ? tftShrinkColor555(trColor888) : 0;
  char hash[5];
  sprintf(hash, "%04X", trColor);
  strcat(extendedName, hash);
  if (!strncmp(tft_bmp.name, extendedName, 17) && tft_bmp.img) return; // Exit if it's already selected
  uint8_t *found = tftFindObject(extendedName);
  if (!found) found = tftLoadBMP(name, extendedName, trColor);
  if (!found) {
    tft_bmp.img = 0;
    tft_bmp.name[0] = 0;
  } else {
    strncpy(tft_bmp.name, extendedName, 17);
    tft_bmp.w = MEM16(found);
    tft_bmp.h = MEM16(found + 2);
    tft_bmp.trColor = trColor;
    tft_bmp.img = found + 4;
  }  
}

#if 0

void tft_line(uint16_t px1, uint16_t py1, uint16_t px2, uint16_t py2) {
  uint16_t x1,y1,x2,y2;
  if ((x1 = px1) > TFT_W) return;
  if ((x2 = px2) > TFT_W) return;
  if ((y1 = py1) > TFT_H) return;
  if ((y2 = py2) > TFT_H) return;
  uint16_t mlen, slen;
  uint32_t mstp, sstp;
  if (x1 <= x2) {
    mlen = x2 - x1;
    mstp = 1;
  } else {
    mlen = x1 - x2;
    mstp = (uint32_t)(-1);
  }    
  if (y1 <= y2) {
    slen = y2 - y1;
    sstp = TFT_W;
  } else {
    slen = y1 - y2;
    sstp = (uint32_t)(-TFT_W);
  }
  uint16_t *pix = (uint16_t *)tft_addr + x1 + y1 * TFT_W;
  if (mlen < slen) {
    swap_u16(mlen, slen);
    swap_u32(mstp, sstp);
  }
  uint16_t cnt = mlen >> 1;
  uint16_t color = ((tft_fg & 0x0000F8) >> 3) | ((tft_fg & 0x00FC00) >> 5) | ((tft_fg & 0xF80000) >> 8);
  uint8_t pos = 0;
  for (uint16_t i=mlen+1; i; i--) {
    if (tft_line_msk & tft_line_msk_point) *pix = color;
    pix += mstp;
    if ((cnt += slen) >= mlen) {
      cnt -= mlen;
      pix += sstp;
      pos += 38;
      if (pos & 0x80) {
        tft_line_msk_point <<= 1;
        if (tft_line_msk_point == 0) tft_line_msk_point = 1;
        pos &= 0x7f;
      }  
    }
    tft_line_msk_point <<= 1;
    if (tft_line_msk_point == 0) tft_line_msk_point = 1;
    pos &= 0x7f;
  }  
}

//https://electronix.ru/forum/lofiversion/index.php/t143598-50.html

void tft_draw_bmp(int16_t x, int16_t y) {
  uint16_t w = bmp_w;
  uint16_t h = bmp_h;
  uint16_t bmp_x = 0;
  uint16_t bmp_y = 0;
  if (bmp_addr == 0) return;
  if (x >= TFT_WIDTH) return;
  if (y >= TFT_HEIGHT) return;
  if ((x + w) < 1) return;
  if ((y + h) < 1) return;
  if ((x + w) > TFT_WIDTH) w = TFT_WIDTH - x;
  if ((y + h) > TFT_HEIGHT) h = TFT_HEIGHT - y;
  if (x < 0) { w += x; bmp_x = -x; x = 0; }
  if (y < 0) { h += y; bmp_y = -y; y = 0; }
  uint32_t s_addr = tft_addr + TFT_PIXEL * (y * TFT_WIDTH + x);
  uint32_t i_addr = (uint32_t)bmp_addr + TFT_PIXEL * (bmp_y * bmp_w + bmp_x);
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
}  

#endif
