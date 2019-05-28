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

//void cmpCreateMenu(TListMenu *mnu, const char *text, TMenuItem) {

#define SYS_BG      0x000088
#define ITEM_FG     0xffffff
#define SEL_BG      0xffff00
#define SEL_FG      0x0000ff
#define ITEM_FNT    SF_12x24
#define ITEM_FNT_W  12
#define ITEM_FNT_H  24
#define ITEM_STEP   28
#define LINES_CNT   8
#define X0          10
#define Y0          38
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
  uint8_t limit = mnu->state->first + LINES_CNT;
  if (limit > mnu->state->count) limit = mnu->state->count; 
  for (uint8_t i = mnu->state->first ; i < limit ; i++) { // Items loop
    if (i == mnu->state->current) {  // Draw selection and set text color
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
    y += ITEM_STEP;
  }
}

void cmpMenuUserInput(TListMenu *mnu, uint8_t key) {
  TListMenuState *state = mnu->state;
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
    const TMenuItem *item = &mnu->item[state->current];
    if (item->onEnter) item->onEnter();
    break; }
  }
}

/*  
  scrResetPoint(0);
  scrCLS(SYS_MNU_BG);
  int16_t x = (TFT_WIDTH - SYS_HDR_FONT_WIDTH * strlen(header)) / 2;
  scrSetFG(SYS_HDR_FG);
  scrSetFontStatic(SYS_HDR_FONT);
  scrSetTextPos(x, 0);
  scrTextOutStatic(header);
  scrSetFontStatic(SYS_MNU_FONT);
  int16_t y = TFT_HEIGHT - SYS_MNU_FONT_HEIGHT;
  scrSetTextPos(5, y);
  scrTextOutStatic(status);
*/  
  
#ifdef __nothnin__
void cmpCreateMenuItems(TListMenu *mnu, const char *text) {
  uint8_t len;
  TMenuItem *last_item;
  while (len = strlen(text)) {
    TMenuItem *item = malloc(sizeof(TMenuItem));
    if (mnu->item_count == 0) {
      mnu->head_item = item;
    } else {
      last_item->next = item;
    }
    mnu->item_count++;
    last_item = item;
    item->next = 0;
    item->onEnter = 0;
    item->type = 0;
    item->value = 0;
    item->text = text;
    text += (len + 1);
  }
}

TMenuItem *cmpMenuItem(TListMenu *mnu, uint8_t index) {
  TMenuItem *result = mnu->head_item;
  for (uint8_t i = 0; i < index; i++) {
    result = result->next;
  }
  return result;
}

void cmpShowItem(TMenuItem *item, uint8_t x, uint8_t y) {
  scrSetTextPos(x, y);
  scrTextOut(item->text);
 
}

/* Draw menu list using scr functions
 */
void cmpShowMenu(TListMenu *mnu) {
  scrSetFontStatic(mnu->font);  // Prepare drawing visible items
  uint8_t item_index = mnu->first_item;
  uint8_t y = mnu->y;
  uint8_t item_x = mnu->x + mnu->font_width / 2;
  uint8_t text_x = item_x;
  TMenuItem *item = cmpMenuItem(mnu, item_index);
  for (uint8_t i = 0; item && i < mnu->lines; i++, item_index++) { // Draw items
    if (item_index == mnu->cur_item) {  // Draw selection and set text color
      scrSetBG(mnu->bg_selected);
      scrBar(mnu->x, y, mnu->width, mnu->step);
      scrSetFG(mnu->fg_selected);
    } else {
      scrSetFG(mnu->fg_color);
    }
    if (mnu->show_numbers) {    // Show item number and update x
      char num[4];
      sprintf(num, "%hu", item_index + 1);
      scrSetTextPos(item_x, y);
      scrTextOut(num);
      text_x = item_x + mnu->font_width * (strlen(num) + 1);
    }
    cmpShowItem(item, text_x, y);
    y += mnu->step;
    item = item->next;
  }
  if (mnu->first_item != 0) {  // Top scroll line
    scrSetBG(mnu->bg_selected);
    scrSetFG(mnu->bg_color);
    scrSetLinePattern(0xCCCCCCCC);
    scrSetLineWidth(1);
    scrBar(mnu->x, mnu->y, mnu->width, 1);
    scrLine(mnu->x, mnu->y, mnu->x + mnu->width, mnu->y);
  }
  if (mnu->first_item + mnu->lines < mnu->item_count) {  // Bottom scroll line
    scrSetBG(mnu->bg_selected);
    scrSetFG(mnu->bg_color);
    scrSetLinePattern(0xCCCCCCCC);
    scrSetLineWidth(1);
    uint16_t bottom = mnu->y + mnu->lines * mnu->step;
    scrBar(mnu->x, bottom, mnu->width, 1);
    scrLine(mnu->x, bottom, mnu->x + mnu->width, bottom);
  }
}

void cmpMenuUserInput(TListMenu *mnu, uint8_t key) {
  switch (key) {
  case KEY_UP:
    if (mnu->cur_item == 0) break;
    mnu->cur_item--;
    uiNeedUpdate = 1;
    break;
  case KEY_DOWN:
    if (mnu->cur_item >= mnu->item_count - 1) break;
    mnu->cur_item++;
    uiNeedUpdate = 1;
    break;
  case KEY_ENTER: {
    TMenuItem *item = cmpMenuItem(mnu, mnu->cur_item);
    if (item->onEnter) item->onEnter();
    break; }
  }
}

#endif
