/******************************************************************************
 *  Components for setup menus
 ******************************************************************************/
#include "laa_components.h"
#include "stdlib.h"
#include "string.h"
#include "laa_scr_tasks.h"
#include "laa_interface.h"
#include "laa_keyboard.h"
  
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
   
