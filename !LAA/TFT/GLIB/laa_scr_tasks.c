/******************************************************************************
 *  Sceduler for TFT output tasks
 ******************************************************************************/
#include "laa_scr_tasks.h"
#include "laa_tft_lib.h"
#include "laa_tft_buffers.h"
#include "laa_global_utils.h"

#define  SCR_BUF_SIZE 16384
uint8_t  scr_task[SCR_BUF_SIZE];
uint16_t scr_end = 0;
uint16_t scr_pnt = 1;
uint16_t scr_new_data_to_show = 0;
uint16_t scr_mark[16];

void impSetBG();
void impSetFG();
void impBar();

#define  SCM_COUNT  (sizeof(imp_routine)/sizeof(imp_routine[0]))
void (*imp_routine[])() = {
  impSetBG,
  impSetFG,
  impBar
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

#define  SCM_SET_BG             0    // color (24)     L=4
#define  SCM_SET_FG             1    // color (24)     L=4
#define  SCM_BAR                2    // x,y,w,h (16x4) L=9

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
  if (mark == 15) return;
    if (mark > 16) return;
  scr_mark[mark-1] = scr_pnt;
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
void scrSetBG(uint32_t color) {
  push_uint8(SCM_SET_BG);
  push_uint24(color);
}

void scrSetFG(uint32_t color) {
  push_uint8(SCM_SET_FG);
  push_uint24(color);
}

void scrBar(int16_t x, int16_t y, int16_t w, int16_t h) {
  push_uint8(SCM_BAR);
  push_uint16(x);
  push_uint16(y);
  push_uint16(w);
  push_uint16(h);
}  

//----- Task implementation rutines ---
void impSetBG() {
  tftSetBackground(get_uint24());
}  

void impSetFG() {
  tftSetForeground(get_uint24());
}  

void impBar() {
  int16_t x = get_uint16(); 
  int16_t y = get_uint16(); 
  int16_t w = get_uint16(); 
  int16_t h = get_uint16();
  tftRect(x, y, w, h);
}  
