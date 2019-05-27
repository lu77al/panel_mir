/******************************************************************************
 *  Setup menus and so on
 ******************************************************************************/
#include "laa_setup.h"
#include "laa_components.h"
#include "laa_scr_tasks.h"
#include "laa_tft_lib.h"
#include "laa_interface.h"
#include "string.h"
#include "stdlib.h"

#define SYS_MNU_BG      0x000088
#define SYS_MNU_FG      0xffffff
#define SYS_SEL_BG      0xffff00
#define SYS_SEL_FG      0x0000ff
#define SYS_MNU_FONT    SF_12x24
#define SYS_MNU_FONT_HEIGHT 24
#define SYS_HDR_FG      0x88ff88
#define SYS_HDR_FONT    SF_16x32
#define SYS_HDR_FONT_WIDTH 16

TListMenu root_menu;
TListMenu test_menu;
   
TListMenu *active_menu = &root_menu;  

void stpFillMenuTemplate(TListMenu *mnu);
void stpPrepareSystemScreen(const char *header, const char *status);
void stpMenuInput(uint8_t key);
void stpTestMenuShow();

static const char menu_hint[] = "Вверх/Вниз: Выбор; Enter: Пуск";

// -------------- Root menu ----------------
static const char root_menu_header[] = "K1021 / V2.0 / 21.05.2019";
static const char root_menu_hint[] = "Вверх/Вниз; Enter: Пуск; <- Назад";
  
static const char root_menu_lines[] =
  "Запустить проект\x00"
  "Связь с ПК (K753)\x00"
  "Тесты\x00"
  "Настройка\x00";

volatile uint16_t lll;
   
void stpStartProject() {
  root_menu.cur_item = 0;
  uiNeedUpdate = 1;
}

void stpConnectPC() {
  root_menu.cur_item = 0;
  uiNeedUpdate = 1;
}

void stpEnterTests() {
  active_menu = &test_menu;
  uiDrawScreenRoutine = stpTestMenuShow;
  uiNeedUpdate = 1;
}

void stpEnterSetup() {
  root_menu.cur_item = 0;
  uiNeedUpdate = 1;
}

void stpRootMenuCreate() {
  TListMenu *mnu = &root_menu;
  stpFillMenuTemplate(mnu);
  cmpCreateMenuItems(mnu, root_menu_lines);
  TMenuItem *item = mnu->head_item;
  item->onEnter = stpStartProject; // Запустить проект
  item = item->next;
  item->onEnter = stpConnectPC; // Связь с ПК (K753)
  item = item->next;
  item->onEnter = stpEnterTests; // Тесты
  item = item->next;
  item->onEnter = stpEnterSetup; // Настройка
}  
   
void stpRootMenuShow() {
  stpPrepareSystemScreen(root_menu_header, root_menu_hint); 
  cmpShowMenu(&root_menu);
}

// -------------- Root menu ----------------
static const char test_menu_header[] = "K1021 -> ТЕСТЫ";
static const char test_menu_lines[] =
  "Тест клавиатуры\x00"
  "Тест портов\x00"
  "Тест дисплея\x00"
  "Тест карты памяти\x00"
  "Тест ОЗУ\x00";

void stpTestMenuCreate() {
  TListMenu *mnu = &test_menu;
  stpFillMenuTemplate(mnu);
  cmpCreateMenuItems(mnu, test_menu_lines);
}  

void stpTestMenuShow() {
  stpPrepareSystemScreen(test_menu_header, menu_hint); 
  cmpShowMenu(&root_menu);
}
   
// ------------ Common routines -------------
void stpMenuInput(uint8_t key) {
  cmpMenuUserInput(active_menu, key);
}

void stpPrepareSystemScreen(const char *header, const char *status) {
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
}


void stpInitSetup() {
  stpRootMenuCreate();
  stpTestMenuCreate();
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
  mnu->step = SYS_MNU_FONT_HEIGHT + 4;
  mnu->font_width = 12;
  mnu->lines = 8;
  mnu->show_numbers = 1;
  mnu->cur_item = 0;
  mnu->first_item = 0;
}

