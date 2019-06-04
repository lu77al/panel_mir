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

void cmpListMenuActivate(TListMenu *mnu); // Set active_menu, draw and user_input handlers

void cmpLogMemoActivate(const char *header, const char *status); // Set Header/Status, reset state, set draw handler
void cmpLMPrint(const char *text);
void cmpLMPrintColor(const char *text, uint32_t color);
void cmpLMNextLine();
void cmpLogMemoForceUpdate();

#endif // __AL_COMPONENTS_H__
