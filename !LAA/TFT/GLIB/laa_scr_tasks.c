/******************************************************************************
 *  Sceduler for TFT output tasks
 ******************************************************************************/
#include "laa_scr_tasks.h"
#include "string.h"
#include "laa_tft_lib.h"
#include "laa_tft_buffers.h"
#include "laa_global_utils.h"

#define  SCR_BUF_SIZE 16384
uint8_t  scr_task[SCR_BUF_SIZE];
uint16_t scr_end = 0;
uint16_t scr_pnt = 1;
uint16_t scr_poly_vcnt_pnt = 0;
uint16_t scr_poly_pnt = 0;
uint16_t scr_new_data_to_show = 0;
uint16_t scr_mark[16];

void impGoDoubleBuffered();
void impGoSingleBuffered();
void impSetBG();
void impSetFG();
void impClearScreem();
void impBar();
void impSetPenWidth();
void impSetPenPattern();
void impMoveTo();
void impLineTo();
void impLineRel();
void impLine();
void impSetFontStatic();
void impSetFontDynamic();
void impSetTextTransparancy();
void impSetTextPos();
void impTextStatic();
void impTextDynamic();
void impPoly();
void impEllipse();
void impSetBMPstatic();
void impSetBMPdynamic();
void impDrawBMP();

#define  SCM_COUNT  (sizeof(imp_routine)/sizeof(imp_routine[0]))
void (*imp_routine[])() = {
  impGoDoubleBuffered,
  impGoSingleBuffered,
  impSetBG,
  impSetFG,
  impClearScreem,
  impBar,
  impSetPenWidth,
  impSetPenPattern,
  impMoveTo,
  impLineTo,
  impLineRel,
  impLine,
  impSetFontStatic,
  impSetFontDynamic,
  impSetTextTransparancy,
  impSetTextPos,
  impTextStatic,
  impTextDynamic,
  impPoly,
  impEllipse,
  impSetBMPstatic,
  impSetBMPdynamic,
  impDrawBMP
};

//----- Task stack management ------
void push_uint8(uint8_t data) {
  if (scr_end >= SCR_BUF_SIZE) return;
  scr_task[scr_end++] = data;
}  

void push_uint16(uint16_t data) {
  if (scr_end >= SCR_BUF_SIZE - 1) return;
  scr_task[scr_end++] = data;
  scr_task[scr_end++] = data >> 8;
}  

void push_uint24(uint32_t data) {
  if (scr_end >= SCR_BUF_SIZE - 2) return;
  scr_task[scr_end++] = data;
  scr_task[scr_end++] = data >> 8;
  scr_task[scr_end++] = data >> 16;
}  

void push_uint32(uint32_t data) {
  if (scr_end >= SCR_BUF_SIZE - 3) return;
  scr_task[scr_end++] = data;
  scr_task[scr_end++] = data >> 8;
  scr_task[scr_end++] = data >> 16;
  scr_task[scr_end++] = data >> 24;
}  

uint8_t get_uint8() {
  if (scr_pnt >= scr_end) return 0;
  return scr_task[scr_pnt++];
}  

uint16_t get_uint16() {
  uint16_t res =  scr_task[scr_pnt++];
  res += scr_task[scr_pnt++] << 8;
  return res;
}  

uint32_t get_uint24() {
  uint32_t res =  scr_task[scr_pnt++];
  res += scr_task[scr_pnt++] << 8;
  res += scr_task[scr_pnt++] << 16;
  return res;
}  

uint32_t get_uint32() {
  uint32_t res =  scr_task[scr_pnt++];
  res += scr_task[scr_pnt++] << 8;
  res += scr_task[scr_pnt++] << 16;
  res += scr_task[scr_pnt++] << 24;
  return res;
}  

#define  SCM_GO_DOUBLE          0
#define  SCM_GO_SINGLE          1
#define  SCM_SET_BG             2
#define  SCM_SET_FG             3
#define  SCM_CLS                4
#define  SCM_BAR                5
#define  SCM_PEN_WIDTH          6
#define  SCM_PEN_STYLE          7
#define  SCM_MOVE_TO            8
#define  SCM_LINE_TO            9
#define  SCM_LINE_REL           10
#define  SCM_LINE               11
#define  SCM_FONT_STATIC        12
#define  SCM_FONT_DYNAMIC       13
#define  SCM_TRANSPARENVY       14
#define  SCM_TEXT_POS           15
#define  SCM_TEXT_STATIC        16
#define  SCM_TEXT_DYNAMIC       17
#define  SCM_POLY               18
#define  SCM_ELLIPSE            19
#define  SCM_BMP_STATIC         20
#define  SCM_BMP_DYNAMIC        21
#define  SCM_BMP                22

//---- Task manager ----
void scrResetPnt(uint8_t mark) {
  if ((mark == 0) || (mark > 16)) {
    scr_end = 0;
  } else {
    scr_end = scr_mark[mark-1];
  }  
  scr_new_data_to_show = 0;
  scr_pnt = 0;
}  

void scrSaveMark(uint8_t mark) {
  if (mark == 0) return;
    if (mark > 16) return;
  scr_mark[mark-1] = scr_end;
}  

void scrSetNewDataFlag() {
  scr_new_data_to_show = 1;
}  

uint8_t scrNeedNewContent() {
  return (scr_new_data_to_show && (scr_pnt > scr_end));
}

void scrPerformNextTask() {
  if (tftIsBusy()) return;
  if (tftIsTaskToDo()) {
    tftDoTheNext();
    return;
  }
  if (scr_pnt >= scr_end) {
    if (scr_pnt == scr_end) {
      scr_pnt++;
      tftNextFrame();
    }  
    return;
  }  
  if (tftIsWaitingForReload()) return;
  uint8_t cmd = get_uint8(); 
  if (cmd >= SCM_COUNT) {
    scr_pnt = scr_end + 1;
    return;
  }  
  imp_routine[cmd]();
}  

//----- Task setup rutines ---
void scrGoDouble() {
  push_uint8(SCM_GO_DOUBLE);
}

void scrGoSingle() {
  push_uint8(SCM_GO_SINGLE);
}

void scrSetBG(uint32_t color) {
  push_uint8(SCM_SET_BG);
  push_uint24(color);
}

void scrSetFG(uint32_t color) {
  push_uint8(SCM_SET_FG);
  push_uint24(color);
}

void scrCLS(int32_t color) {
  push_uint8(SCM_CLS);
  push_uint24(color);
}  

void scrBar(int16_t x, int16_t y, int16_t w, int16_t h) {
  push_uint8(SCM_BAR);
  push_uint16(x);
  push_uint16(y);
  push_uint16(w);
  push_uint16(h);
}  

void scrSetPenWidth(int8_t width) {
  push_uint8(SCM_PEN_WIDTH);
  push_uint8(width);
}  

void scrSetPenPattern(int32_t pattern) {
  push_uint8(SCM_PEN_STYLE);
  push_uint32(pattern);
}  

void scrMoveTo(int16_t x, int16_t y) {
  push_uint8(SCM_MOVE_TO);
  push_uint16(x);
  push_uint16(y);
}  

void scrLineTo(int16_t x, int16_t y) {
  push_uint8(SCM_LINE_TO);
  push_uint16(x);
  push_uint16(y);
}  

void scrLineRel(int16_t x, int16_t y) {
  push_uint8(SCM_LINE_REL);
  push_uint16(x);
  push_uint16(y);
}  

void scrLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  push_uint8(SCM_LINE);
  push_uint16(x1);
  push_uint16(y1);
  push_uint16(x2);
  push_uint16(y2);
}  

void scrSetFontStatic(char *name) {
  push_uint8(SCM_FONT_STATIC);
  push_uint32((uint32_t)name);
}

void scrSetFontDynamic(char *name) {
  push_uint8(SCM_FONT_DYNAMIC);
  char *src = name;
  for (uint8_t i=13; i; i--) {
    if (!*src) break;
    push_uint8(*(src++));
  }  
  push_uint8(0);
}

void scrSetTextTransparency(int8_t transparent) {
  push_uint8(SCM_TRANSPARENVY);
  push_uint8(transparent);
}

void scrSetTextPos(int16_t x, int16_t y) {
  push_uint8(SCM_TEXT_POS);
  push_uint16(x);
  push_uint16(y);
}

void scrTextOutStatic(void *text, uint8_t maxLen) {
  push_uint8(SCM_TEXT_STATIC);
  push_uint8(maxLen);
  push_uint32((uint32_t)text);
}

void scrTextOutDynamic(void *text, uint8_t maxLen) {
  push_uint8(SCM_TEXT_DYNAMIC);
  uint8_t len = strlen(text);
  if (maxLen > len) maxLen = len;
  push_uint8(maxLen);
  memcpy(&scr_task[scr_end], text, maxLen);
  scr_end += maxLen;
}

void scrInitPoly(uint8_t filled, uint8_t closed) {
  push_uint8(SCM_POLY);
  scr_poly_vcnt_pnt = scr_end;
  push_uint8(0);
  uint8_t flags = filled ? 1 : 0;
  if (closed) flags |= 2;
  push_uint8(flags);
  scr_poly_pnt = scr_end;
}  

void scrPolyVertex(int16_t x, int16_t y) {
  if (scr_poly_pnt != scr_end) return;
  push_uint16(x);
  push_uint16(y);
  scr_task[scr_poly_vcnt_pnt]++;
  scr_poly_pnt = scr_end;
}  

void scrEllipse(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t s, uint16_t e, uint8_t filled, uint8_t closed) {
  push_uint8(SCM_ELLIPSE);
  push_uint16(x);
  push_uint16(y);
  push_uint16(w);
  push_uint16(h);
  push_uint16(s);
  push_uint16(e);
  push_uint8(filled);
  push_uint8(closed);
}  

void scrSetBMPstatic(char *name, uint32_t trColor888) {
  push_uint8(SCM_BMP_STATIC);
  push_uint32(trColor888);
  push_uint32((uint32_t)name);
}

void scrSetBMPdynamic(char *name, uint32_t trColor888) {
  push_uint8(SCM_BMP_DYNAMIC);
  push_uint32(trColor888);
  char *src = name;
  for (uint8_t i=13; i; i--) {
    if (!*src) break;
    push_uint8(*(src++));
  }  
  push_uint8(0);
}

void scrDrawBMP(int16_t x, int16_t y, uint8_t alpha) {
  push_uint8(SCM_BMP);
  push_uint16(x);
  push_uint16(y);
  push_uint8(alpha);
}

//----- Task implementation rutines ---

void impGoDoubleBuffered() {
  tftGoDoubleBuffered();
}

void impGoSingleBuffered() {
  tftGoSingleBuffered();
}

void impSetBG() {
  tftSetBackground(get_uint24());
}  

void impSetFG() {
  tftSetForeground(get_uint24());
}  

void impClearScreem() {
  tftClearScreen(get_uint24());
}  

void impBar() {
  int16_t  x = get_uint16();
  int16_t  y = get_uint16();
  uint16_t w = get_uint16();
  uint16_t h = get_uint16();
  tftRect(x, y, w, h);
}  

void impSetPenWidth() {
  tftSetPenWidth(get_uint8()); 
}  

void impSetPenPattern() {
  tftSetPenPattern(get_uint32());
}  

void impMoveTo() {
  int16_t  x = get_uint16();
  int16_t  y = get_uint16();
  tftMoveTo(x, y);  
}  

void impLineTo() {
  int16_t  x = get_uint16();
  int16_t  y = get_uint16();
  tftLineTo(x, y);  
}  

void impLineRel() {
  int16_t  x = get_uint16();
  int16_t  y = get_uint16();
  tftLineRel(x, y);  
}  

void impLine() {
  int16_t  x1 = get_uint16();
  int16_t  y1 = get_uint16();
  int16_t  x2 = get_uint16();
  int16_t  y2 = get_uint16();
  tftLine(x1, y1, x2, y2);
}  

void impSetFontStatic() {
  tftSelectFont((char *)get_uint32());
}  

void impSetFontDynamic() {
  tftSelectFont((char *)&scr_task[scr_pnt]);
  while (scr_task[scr_pnt]) scr_pnt++;
  scr_pnt++;
}  

void impSetTextTransparancy() {
   tftSetTextTransparency(get_uint8());
}  

void impSetTextPos() {
  int16_t  x = get_uint16();
  int16_t  y = get_uint16();
  tftSetTextPos(x, y);
}  

void impTextStatic() {
  uint8_t maxLen = get_uint8();
  tftTextOut((char *)get_uint32(), maxLen);
}  

void impTextDynamic() {
  uint8_t maxLen = get_uint8();
  tftTextOut((char *)&scr_task[scr_pnt], maxLen);
  scr_pnt += maxLen;
}  

void impPoly() {
  uint8_t vCnt = get_uint8();
  uint8_t flags = get_uint8();
  tftPolyInit(flags & 1, flags & 2);
  int16_t x;
  int16_t y;
  for (;vCnt; vCnt--) {
    x = get_uint16();
    y = get_uint16();
    tftPolyAddVertex(x, y);
  }  
}

void impEllipse() {
  int16_t   x = get_uint16();
  int16_t   y = get_uint16();
  uint16_t  w = get_uint16();
  uint16_t  h = get_uint16();
  uint16_t  s = get_uint16();
  uint16_t  e = get_uint16();
  uint8_t   f = get_uint8();
  uint8_t   c = get_uint8();
  tftEllipse(x, y, w, h, s, e, f, c);
}  

void impSetBMPstatic() {
  uint32_t TrColor = get_uint32();
  char *name = (char *)get_uint32();  
  tftSelectBMP(name, TrColor);
}  

void impSetBMPdynamic() {
  uint32_t TrColor = get_uint32();
  tftSelectBMP((char *)&scr_task[scr_pnt], TrColor);
  while (scr_task[scr_pnt]) scr_pnt++;
  scr_pnt++;
}  

void impDrawBMP() {
  int16_t  x = get_uint16();
  int16_t  y = get_uint16();
  uint8_t  alpha = get_uint8();
  tftDrawBMP(x, y, alpha);  
}  
