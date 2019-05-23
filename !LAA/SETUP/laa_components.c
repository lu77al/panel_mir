/******************************************************************************
 *  Components for setup menus
 ******************************************************************************/
#include "laa_components.h"
#include "stdlib.h"
#include "string.h"
#include "laa_scr_tasks.h"

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

void cmpShowItem(TMenuItem *mnu) {
}

void cmpShowMenu(TListMenu *mnu) {
  scrSetBG(mnu->bg_color);
  uint16_t height = mnu->lines * mnu->step;
  scrBar(mnu->x, mnu->y, mnu->width, height);
  scrSetBG(mnu->bg_selected);
  uint16_t selected_y = mnu->y + (mnu->cur_item - mnu->first_item) * mnu->step;
  scrBar(mnu->x, selected_y, mnu->width, mnu->step);
  scrSetFontStatic(mnu->font);
  uint8_t item_index = mnu->first_item;
  uint8_t y = mnu->y;
  TMenuItem *item = cmpMenuItem(mnu, item_index);
  for (uint8_t i = 0; item && i < mnu->lines; i++, item_index++) {
    scrSetFG(item_index == mnu->cur_item ? mnu->fg_selected : mnu->fg_color);
    char num[4];
    sprintf(num, "%hu", item_index + 1);
    scrSetTextPos(mnu->x, y);
    scrTextOut(num, 3);
    y += mnu->step;
    item = item->next;
  }
}

   
//char* stp_menu_content = 0;
//void stpSetMenuContent
/*
  M



*/


void stpIdeasTest() {
  uint16_t* smth = malloc(10);
//  strcasecmp(
}  


/* Draw menu items from string buffer, selection
 */
void stpDrawMenuItems() {
//  if (!stp_menu_content) return;
  
}