#ifndef __AL_COMPONENTS_H__
#define __AL_COMPONENTS_H__

#include "stm32f7xx_hal.h"

typedef struct TMenuValue {
  uint8_t    *value;
  uint8_t    min;
  uint8_t    max;
  const char *text;
  void      (*onChange)();
} TMenuValue;

typedef struct TMenuItem {
  const char *text;
  void      (*onEnter)();
  TMenuValue *value;
  void       *extended;
} TMenuItem;

typedef struct TListMenuState {
  uint8_t    count;
  uint8_t    first;
  uint8_t    current;
} TListMenuState;

typedef struct TListMenu {
  const char       *header;
  const char       *status;
  const TMenuItem  *item;
  void      (*onExit)();
  uint16_t         selection_width;
  TListMenuState   *state;
} TListMenu;

void cmpDrawMenu(TListMenu *mnu);
void cmpMenuUserInput(TListMenu *mnu, uint8_t key);

/*
void cmpCreateMenuItems(TListMenu *mnu, const char *text);
void cmpShowMenu(TListMenu *mnu);
void cmpMenuUserInput(TListMenu *mnu, uint8_t key);
TMenuItem *cmpMenuItem(TListMenu *mnu, uint8_t index);
*/

#endif // __AL_COMPONENTS_H__
