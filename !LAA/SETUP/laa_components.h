#ifndef __AL_COMPONENTS_H__
#define __AL_COMPONENTS_H__

#include "stm32f7xx_hal.h"

#define SYS_MNU_BG      0x000088

typedef struct TMenuItem {
  struct TMenuItem *next;
  uint8_t type;
  const char *text;
  void       *value;
  void      (*onEnter)();
} TMenuItem;

typedef struct TListMenu{
  uint16_t  x, y; // Position
  const char *font;
  uint16_t  width;
  uint8_t   lines, step;
  uint8_t   show_numbers;
  uint32_t  bg_color, fg_color;
  uint32_t  bg_selected, fg_selected;
  uint8_t   item_count;
  uint8_t   cur_item;
  uint8_t   first_item;
  TMenuItem *head_item;
} TListMenu;

void cmpCreateMenuItems(TListMenu *mnu, const char *text);
void cmpShowMenu(TListMenu *mnu);

#endif // __AL_COMPONENTS_H__
