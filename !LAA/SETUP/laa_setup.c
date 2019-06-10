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
#include "laa_config.h"
#include "string.h"
#include "stdlib.h"

void stpEnterTests();
void stpExitTests();
void stpEnterSetup();
void stpExitSetup();
void stpTestKeybordStart();
void stpRunServiceProtocol();
void stpChangeProfile();

TListMenu root_menu = {
  .header = "K1021 / V2.0 / 21.05.2019",
  .status = "�����/����: �����; Enter: ����",
  .item = (TMenuItem[]){
    { .text = "��������� ������",  .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "����� � �� (K753)", .onEnter = stpRunServiceProtocol,
      .value = 0, .extended = 0
    },
    { .text = "�����",             .onEnter = stpEnterTests,
      .value = 0, .extended = 0
    },
    { .text = "���������",         .onEnter = stpEnterSetup,
      .value = 0, .extended = 0
    },
    { .text = "�������� ��������", .onEnter = 0,
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
  .header = "K1021 -> �����",
  .status = "�����/����; Enter: ����; <- �����",
  .item = (TMenuItem[]){
    { .text = "���� ����������",    .onEnter = stpTestKeybordStart,
      .value = 0, .extended = 0
    },
    { .text = "���� ������",        .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "���� �������",       .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "���� ����� ������",  .onEnter = 0,
      .value = 0, .extended = 0
    },
    { .text = "���� ���",           .onEnter = 0,
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

/*   
uint8_t prof = 1;
uint8_t bright = 7;
uint8_t sleep = 2;
uint8_t sound = 0;
uint8_t addr = 0;
uint8_t bitrate = 2;
uint8_t buscheck = 1;
*/

TListMenu setup_menu = {
  .header = "K1021 -> ���������",
  .status = "�����/����; Enter: ����; <- �����",
  .item = (TMenuItem[]){
    { .text = "�������� �������",     .onEnter = 0,
      .value = &((TMenuValue){
         .value = &cfg_cur_profile,
         .min = 1,
         .max = 5,
         .text = 0, .onChange = stpChangeProfile
      }),
      .extended = 0
    },
/*    { .text = "������� ���������",    .onEnter = 0,
      .value = &((TMenuValue){
         .value = &bright,
         .min = 1,
         .max = 10,
         .text = 0, .onChange = 0
      }),
      .extended = 0
    }, */
    { .text = "���������� ������",    .onEnter = 0,
      .value = &((TMenuValue){
         .value = &(cfg_profile[0].ScreenSaverTime),
         .min = 0,
         .max = 11,
         .text = "1 ���\x00""2 ���\x00""3 ���\x00""5 ���\x00""10 ���\x00""15 ���\x00"
                 "30 ���\x00""45 ���\x00""1 ���\x00""2 ����\x00""3 ����\x00""�������",
         .onChange = 0
      }),
      .extended = 0
    },
    { .text = "���� ������",          .onEnter = 0,
      .value = &((TMenuValue){
         .value = &(cfg_profile[0].buttonSound),
         .min = 0,
         .max = 1,
         .text = "����\x00""���\x00",
         .onChange = cfgApply
      }),
      .extended = 0
    },
    { .text = "����� ��� �� ModBus",  .onEnter = 0,
      .value = &((TMenuValue){
         .value = &(cfg_profile[0].plcAddress),
         .min = 0,
         .max = 255,
         .text = 0, .onChange = 0
      }),
      .extended = 0
    },
    { .text = "�������� ModBus",      .onEnter = 0,
      .value = &((TMenuValue){
         .value = &(cfg_profile[0].bitRate),
         .min = 0,
         .max = 4,
         .text = "9600\x00""19200\x00""38400\x00""57600\x00""115200",
         .onChange = 0
      }),
      .extended = 0
    },
    { .text = "�������� ModBus",      .onEnter = 0,
      .value = &((TMenuValue){
         .value = &(cfg_profile[0].parityCheck),
         .min = 0,
         .max = 2,
         .text = "��� ��������\x00""�� ��������\x00""�� ����������",
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
  cmpLogMemoActivate("���� ����������",
                     "<- �����");
  uiNextKeyRoutine = stpTestKeyboardOnKey;
}

void stpServceProtocolOnKey(uint8_t key) {
  if (key == KEY_BACK) {
    cmpListMenuActivate(&root_menu);
    spStop();
  }
}

void stpRunServiceProtocol() {
  cmpLogMemoActivate("����� � �� (�753)",
                     "<- �����");
  uiNextKeyRoutine = stpServceProtocolOnKey;
  spRun();
}

void stpEnterTests() {
  cmpListMenuActivate(&tests_menu);
}

void stpExitTests() {
  cmpListMenuActivate(&root_menu);
}

void stpUpdateSetupPointers() {
  TProfile *prf = &cfg_profile[cfg_cur_profile - 1];
  setup_menu.item[1].value->value = &(prf->ScreenSaverTime);
  setup_menu.item[2].value->value = &(prf->buttonSound);
  setup_menu.item[3].value->value = &(prf->plcAddress);
  setup_menu.item[4].value->value = &(prf->bitRate);
  setup_menu.item[5].value->value = &(prf->parityCheck);
}

void stpChangeProfile() {
  stpUpdateSetupPointers();
  cfgApply();
}

void stpEnterSetup() {
  stpUpdateSetupPointers();
  cmpListMenuActivate(&setup_menu);
}

void stpExitSetup() {
  cmpListMenuActivate(&root_menu);
  cfgSave();
  cfgApply();
}
