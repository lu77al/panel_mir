/******************************************************************************
 *  Setup menus and so on
 ******************************************************************************/
#include "laa_setup.h"
#include "laa_components.h"
#include "laa_scr_tasks.h"
#include "string.h"
#include "stdlib.h"

#define SYS_MNU_BG      0x000088
#define SYS_MNU_FG      0xffffff
#define SYS_SEL_BG      0xffff00
#define SYS_SEL_FG      0x0000ff
#define SYS_MNU_FONT    SF_12x24

TListMenu root_menu;   

void stpFillMenuTemplate(TListMenu *mnu);

// -------------- Root menu ----------------
const char sys_menu_lines[] =
  "Запустить проект\x00"
  "Связь с ПК (K753)\x00"
  "Тесты\x00"
  "Настройка\x00"
  "\x00";

void stpRootMenuCreate() {
  TListMenu *mnu = &root_menu;
  stpFillMenuTemplate(mnu);
  cmpCreateMenuItems(mnu, sys_menu_lines);
}  

void stpRootMenuShow() {
  cmpShowMenu(&root_menu);
}

// ------------ Common rootines -------------
void stpInitSetup() {
  stpRootMenuCreate();
}

void stpFillMenuTemplate(TListMenu *mnu) {
  mnu->bg_color = SYS_MNU_BG;
  mnu->fg_color = SYS_MNU_FG;
  mnu->bg_selected = SYS_SEL_BG;
  mnu->fg_selected = SYS_SEL_FG;
  mnu->font = SYS_MNU_FONT;  
  mnu->x = 10;
  mnu->y = 38;
  mnu->width = 320;
  mnu->step = 28;
  mnu->font_width = 12;
  mnu->lines = 8;
  mnu->show_numbers = 1;
  mnu->cur_item = 0;
  mnu->first_item = 0;
}

