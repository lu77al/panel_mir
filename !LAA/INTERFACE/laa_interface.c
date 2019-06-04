/******************************************************************************
 *  User interface routines calls
 ******************************************************************************/
#include "laa_interface.h"
#include "laa_scr_tasks.h"
#include "laa_keyboard.h"

uint8_t uiNeedUpdate = 0;

void (*uiDrawScreenRoutine)() = 0;
void (*uiNextKeyRoutine)(uint8_t key) = 0;

void (*uiSPNextByte)(uint8_t rxb) = 0;
void (*uiSPProcesMessage)() = 0;

void uiDrawScreen() {
  if (!scrIsRenderComplete()) return;
  if (!uiDrawScreenRoutine) return;
  if (!uiNeedUpdate) return;
  uiNeedUpdate = 0;
  uiDrawScreenRoutine();
  scrStartRender();
}

void uiProcessUserInput() {
  uint8_t key = kbdPollKey();
  if (!key) return;
  if (!uiNextKeyRoutine) return;
  uiNextKeyRoutine(key);
}
