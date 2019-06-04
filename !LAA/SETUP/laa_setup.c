/******************************************************************************
 *  Setup menus and so on
 ******************************************************************************/
#include "laa_setup.h"
#include "laa_components.h"
#include "laa_scr_tasks.h"
#include "laa_tft_lib.h"
#include "laa_interface.h"
#include "laa_keyboard.h"
#include "laa_service_protocol.h"
#include "string.h"
#include "stdlib.h"

void stpEnterTests();
void stpExitTests();
void stpEnterSetup();
void stpExitSetup();
void stpTestKeybordStart();
void stpRunServiceProtocol();

TListMenu root_menu = {
  .header = "K1021 / V2.0 / 21.05.2019",
  .status = "Вверх/Вниз: Выбор; Enter: Пуск",
  .item = (TMenuItem[]){
    { .text = "Запустить проект",  .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "Связь с ПК (K753)", .onEnter = stpRunServiceProtocol,
      .value = 0, .extended = 0
    },
    { .text = "Тесты",             .onEnter = stpEnterTests,
      .value = 0, .extended = 0
    },
    { .text = "Настройка",         .onEnter = stpEnterSetup,
      .value = 0, .extended = 0
    },
    { .text = "Файловый менеджер", .onEnter = 0,
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
    { .text = "Тест клавиатуры",    .onEnter = stpTestKeybordStart,
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
         .text = "Откл\x00""Вкл\x00",
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

void stpStartSetup() {
  cmpListMenuActivate(&root_menu);
}

void stpTestKeyboardOnKey(uint8_t key) {
  if (key == KEY_BACK) {
    cmpListMenuActivate(&tests_menu);
  } else {
    cmpLMPrint(" ");
    cmpLMPrint(kbdGetKeyName(key));
    cmpLMPrint(" ");
  }
}

void stpTestKeybordStart() {
  cmpLogMemoActivate("Тест клавиатуры",
                     "<- выход");
  uiNextKeyRoutine = stpTestKeyboardOnKey;
}

void stpServceProtocolOnKey(uint8_t key) {
  if (key == KEY_BACK) {
    cmpListMenuActivate(&root_menu);
    spStop();
  }
}

void stpRunServiceProtocol() {
  cmpLogMemoActivate("СВЯЗЬ С ПК (К753)",
                     "<- выход");
  uiNextKeyRoutine = stpServceProtocolOnKey;
  spRun();
}

void stpEnterTests() {
  cmpListMenuActivate(&tests_menu);
}

void stpExitTests() {
  cmpListMenuActivate(&root_menu);
}

void stpEnterSetup() {
  cmpListMenuActivate(&setup_menu);
}

void stpExitSetup() {
  cmpListMenuActivate(&root_menu);
}
