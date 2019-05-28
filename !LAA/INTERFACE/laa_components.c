/******************************************************************************
 *  Components for setup menus
 ******************************************************************************/
#include "laa_components.h"
#include "stdlib.h"
#include "string.h"
#include "laa_scr_tasks.h"
#include "laa_interface.h"
#include "laa_keyboard.h"
#include "laa_tft_lib.h"

#define SYS_BG      0x000088
#define ITEM_FG     0xffffff
#define SEL_BG      0xffff00
#define SEL_FG      0x0000ff
#define ITEM_FNT    SF_12x24
#define ITEM_FNT_W  12
#define ITEM_FNT_H  24
#define ITEM_STEP   28
#define LINES_CNT   7
#define X0          10
#define Y0          38
#define X_VALUE     280
#define HEAD_FNT    SF_16x32
#define HEAD_FNT_W  16
#define HEAD_FG     0x88ff88

void cmpInitMenu(TListMenu *mnu) {
  mnu->state = malloc(sizeof(TListMenuState));
  uint8_t cnt = 0;
  const TMenuItem *item = mnu->item;
  while (item->text) {
    item++;
    cnt++;
  }
  mnu->state->count = cnt;
  mnu->state->current = 0;
  mnu->state->first = 0;
}

void cmpDrawMenu(TListMenu *mnu) {
  if (!mnu->state) cmpInitMenu(mnu); // Check init
  TListMenuState *state = mnu->state;
  scrResetPoint(0);     // Clear screen
  scrCLS(SYS_BG);
  scrSetFG(HEAD_FG);    // Header
  scrSetFontStatic(HEAD_FNT);
  scrSetTextPos((TFT_WIDTH - HEAD_FNT_W * strlen(mnu->header)) / 2, 0);
  scrTextOutStatic(mnu->header);
  scrSetFontStatic(ITEM_FNT); // Status / hint
  scrSetTextPos(ITEM_FNT_W / 2, TFT_HEIGHT - ITEM_FNT_H);
  scrTextOutStatic(mnu->status);
  uint16_t y = Y0;      // Prepare to draw items
  uint8_t limit = state->first + LINES_CNT;
  if (limit > state->count) limit = state->count; 
  for (uint8_t i = state->first ; i < limit ; i++) { // Items loop
    if (i == state->current) {  // Draw selection and set text color
      scrSetBG(SEL_BG);
      scrBar(X0, y, mnu->selection_width, ITEM_STEP);
      scrSetFG(SEL_FG);
    } else {
      scrSetFG(ITEM_FG);
    }
    char num[4];        // Print index
    sprintf(num, "%hu", i + 1);
    scrSetTextPos(X0 + ITEM_FNT_W / 2, y);
    scrTextOut(num);
    
    uint8_t x = X0 + ITEM_FNT_W / 2 + ITEM_FNT_W * (strlen(num) + 1); // Print main text
    scrSetTextPos(x, y);
    scrTextOutStatic(mnu->item[i].text);
    if (mnu->item[i].value) { // Print value
      const TMenuValue *value = mnu->item[i].value;
     scrSetTextPos(X_VALUE, y);
      if (value->text) {
        const char *text = value->text;
        for (uint8_t i = value->min; i < *(value->value); i++) {
          text += strlen(text) + 1;
        }
        scrTextOutStatic(text);
      } else {
        sprintf(num, "%hu", *(value->value));
        scrTextOut(num);
      }
    }
    y += ITEM_STEP;
  }
  if (state->first != 0) {  // Top scroll line
    scrSetFG(ITEM_FG);
    scrSetLinePattern(0x0F0F0F0F);
    scrSetLineWidth(2);
    scrLine(X0, Y0 - 2, X0 + mnu->selection_width, Y0 - 2);
  }
  if (state->count - state->first > LINES_CNT) {  // Bottom scroll line
    scrSetFG(ITEM_FG);
    scrSetLinePattern(0x0F0F0F0F);
    scrSetLineWidth(2);
    scrLine(X0, Y0 + LINES_CNT * ITEM_STEP + 2,
            X0 + mnu->selection_width,  Y0 + LINES_CNT * ITEM_STEP + 2);
  }
}

void cmpMenuUserInput(TListMenu *mnu, uint8_t key) {
  if (!mnu->state) cmpInitMenu(mnu); // Check init
  TListMenuState *state = mnu->state;
  const TMenuItem *item = &mnu->item[state->current];
  TMenuValue *value = item->value;
  switch (key) {
  case KEY_UP:
    if (state->current == 0) break;
    state->current--;
    if (state->first > state->current) {
      state->first = state->current;
    }
    uiNeedUpdate = 1;
    break;
  case KEY_DOWN:
    if (state->current >= state->count - 1) break;
    state->current++;
    if (state->current - state->first >= LINES_CNT) {
      state->first = state->current - LINES_CNT + 1;
    }
    uiNeedUpdate = 1;
    break;
  case KEY_ENTER: {
    if (item->onEnter) item->onEnter();
    break; }
  case KEY_BACK:
    if (mnu->onExit) mnu->onExit();
    break;
  case KEY_LEFT:
    if (!value) break;
    if (*(value->value) <= value->min) {
      *(value->value) = value->max;
    } else {
      (*(value->value))--;
    }
    uiNeedUpdate = 1;
    break;
  case KEY_RIGHT:
    if (!value) break;
    if (*(value->value) >= value->max) {
      *(value->value) = value->min;
    } else {
      (*(value->value))++;
    }
    uiNeedUpdate = 1;
    break;
  }
}
