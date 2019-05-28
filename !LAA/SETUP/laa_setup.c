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

/*

static const char root_text[] =
  "K1021 / V2.0 / 21.05.2019\x00"
  "�����/����; Enter: ����; <- �����\x00"
  "��������� ������\x00"
  "����� � �� (K753)\x00"
  "�����\x00"
  "���������\x00";

static const TMenuItem root_items[] = {
  {.onEnter = 0, .value = 0, .extended = 0},
  {.onEnter = 0, .value = 0, .extended = 0}
};


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
  const TMenuItem   *item;
  uint16_t          selection_width;
  TListMenuState    *state;
} TListMenu;
*/

TListMenu root_menu = {
  .header = "K1021 / V2.0 / 21.05.2019",
  .status = "�����/����; Enter: ����; <- �����",
  .item = (TMenuItem[]){
    { .text = "��������� ������",  .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "����� � �� (K753)", .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "�����",             .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "���������",         .onEnter = 0, // \x00 - Terminator
      .value = 0, .extended = 0
    },
    { .text =  0, .onEnter = 0,   // Terminator
      .value = 0, .extended = 0
    }
  },
  .onExit = 0,
  .selection_width = 300,
  .state = 0  
};    

void stpShowActiveMenu() {
  cmpDrawMenu(&root_menu);
}

void stpMenuInput(uint8_t key) {
  cmpMenuUserInput(&root_menu, key);
}

/*

TListMenu test_menu;
   
TListMenu *active_menu = &root_menu;  

void stpFillMenuTemplate(TListMenu *mnu);
void stpPrepareSystemScreen(const char *header, const char *status);
void stpMenuInput(uint8_t key);
void stpTestMenuShow();

static const char menu_hint[] = "�����/����: �����; Enter: ����";

// -------------- Root menu ----------------
static const char root_menu_header[] = "K1021 / V2.0 / 21.05.2019";
static const char root_menu_hint[] = "�����/����; Enter: ����; <- �����";
  
static const char root_menu_lines[] =
  "��������� ������\x00"
  "����� � �� (K753)\x00"
  "�����\x00"
  "���������\x00";

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
  item->onEnter = stpStartProject; // ��������� ������
  item = item->next;
  item->onEnter = stpConnectPC; // ����� � �� (K753)
  item = item->next;
  item->onEnter = stpEnterTests; // �����
  item = item->next;
  item->onEnter = stpEnterSetup; // ���������
}  
   
void stpRootMenuShow() {
  stpPrepareSystemScreen(root_menu_header, root_menu_hint); 
  cmpShowMenu(&root_menu);
}

// -------------- Root menu ----------------
static const char test_menu_header[] = "K1021 -> �����";
static const char test_menu_lines[] =
  "���� ����������\x00"
  "���� ������\x00"
  "���� �������\x00"
  "���� ����� ������\x00"
  "���� ���\x00";

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
*/
