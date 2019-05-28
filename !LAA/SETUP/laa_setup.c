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

void stpEnterTests();
void stpExitTests();
void stpEnterSetup();
void stpExitSetup();

TListMenu root_menu = {
  .header = "K1021 / V2.0 / 21.05.2019",
  .status = "Вверх/Вниз: Выбор; Enter: Пуск",
  .item = (TMenuItem[]){
    { .text = "Запустить проект",  .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "Связь с ПК (K753)", .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "Тесты",             .onEnter = stpEnterTests,
      .value = 0, .extended = 0
    },
    { .text = "Настройка",         .onEnter = stpEnterSetup,
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

TListMenu tests_menu = {
  .header = "K1021 -> ТЕСТЫ",
  .status = "Вверх/Вниз; Enter: Пуск; <- Назад",
  .item = (TMenuItem[]){
    { .text = "Тест клавиатуры",    .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "Тест портов",        .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "Тест дисплея",       .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "Тест карты памяти",  .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "Тест ОЗУ",           .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text =  0, .onEnter = 0,   // Terminator
      .value = 0, .extended = 0
    }
  },
  .onExit = stpExitTests,
  .selection_width = 300,
  .state = 0  
};    

uint8_t prof = 1;
uint8_t bright = 7;
uint8_t sleep = 2;
uint8_t sound = 0;
uint8_t addr = 0;
uint8_t bitrate = 2;
uint8_t buscheck = 1;

TListMenu setup_menu = {
  .header = "K1021 -> НАСТРОЙКА",
  .status = "Вверх/Вниз; Enter: Пуск; <- Назад",
  .item = (TMenuItem[]){
    { .text = "Активный профиль",     .onEnter = 0,
      .value = &((TMenuValue){
         .value = &prof,
         .min = 1,
         .max = 5,
         .text = 0, .onChange = 0
      }),
      .extended = 0
    },
    { .text = "Яркость подсветки",    .onEnter = 0,
      .value = &((TMenuValue){
         .value = &bright,
         .min = 1,
         .max = 10,
         .text = 0, .onChange = 0
      }),
      .extended = 0
    },
    { .text = "Отключение экрана",    .onEnter = 0,
      .value = &((TMenuValue){
         .value = &sleep,
         .min = 0,
         .max = 11,
         .text = "1 мин\x00""2 мин\x00""3 мин\x00""5 мин\x00""10 мин\x00""15 мин\x00"
                 "30 мин\x00""45 мин\x00""1 час\x00""2 часа\x00""3 часа\x00""никогда",
         .onChange = 0
      }),
      .extended = 0
    },
    { .text = "Звук кнопок",          .onEnter = 0,
      .value = &((TMenuValue){
         .value = &sound,
         .min = 0,
         .max = 1,
         .text =   "Откл\x00""Вкл\x00",
         .onChange = 0
      }),
      .extended = 0
    },
    { .text = "Адрес ModBus",         .onEnter = 0,
      .value = &((TMenuValue){
         .value = &addr,
         .min = 0,
         .max = 255,
         .text = 0, .onChange = 0
      }),
      .extended = 0
    },
    { .text = "Скорость ModBus",      .onEnter = 0,
      .value = &((TMenuValue){
         .value = &bitrate,
         .min = 0,
         .max = 4,
         .text = "9600\x00""19200\x00""38400\x00""57600\x00""115200",
         .onChange = 0
      }),
      .extended = 0
    },
    { .text = "Проверка ModBus",      .onEnter = 0,
      .value = &((TMenuValue){
         .value = &buscheck,
         .min = 0,
         .max = 2,
         .text = "без проверки\x00""на четность\x00""на нечетность",
         .onChange = 0
      }),
      .extended = 0
    },
    { .text =  0, .onEnter = 0,   // Terminator
      .value = 0, .extended = 0
    }
  },
  .onExit = stpExitSetup,
  .selection_width = 445,
  .state = 0  
};    

TListMenu *active_menu = &root_menu;

void stpShowActiveMenu() {
  if (!active_menu) return;
  cmpDrawMenu(active_menu);
}

void stpMenuInput(uint8_t key) {
  if (!active_menu) return;
  cmpMenuUserInput(active_menu, key);
}

void stpEnterTests() {
  active_menu = &tests_menu;
  uiNeedUpdate = 1;
}

void stpExitTests() {
  active_menu = &root_menu;
  uiNeedUpdate = 1;
}

void stpEnterSetup() {
  active_menu = &setup_menu;
  uiNeedUpdate = 1;
}

void stpExitSetup() {
  active_menu = &root_menu;
  uiNeedUpdate = 1;
}
