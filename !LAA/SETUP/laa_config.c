/******************************************************************************
 *  Handling profile directories, save/read settings to SD
 ******************************************************************************/
#include "laa_config.h"
#include "laa_sdcard.h"
#include "laa_sdram.h"
#include "laa_keyboard.h"
#include "ff.h"
#include "fatfs.h"
#include "string.h"

/***** SD Error strategy *****
 *  - Each error -> RESET CPU (with counter of resets)
 *  - After N resets -> show message to reset manually (with option of enetering sysMenu)
 *  - when error occurs on reading setup don't show message, just set it to default
 */

uint8_t         cfg_cur_profile;
TProfile        cfg_profile[5];
static uint8_t  cfg_reset_cnt;
static uint8_t  io_reset_cnt;

void cfgApply();
void cfgDefault();
void cfgSave();

#define IO_RESET  0
#define CFG_RESET 1
#define MAX_CFG_READ_ATTEMPTS 8

#define RESET_MESSAGE   (RESET_DATA + 16)
#define RESET_CFG_CNT   (*(uint8_t *)(RESET_DATA))
#define RESET_SDIO_CNT  (*(uint8_t *)(RESET_DATA + 1))

const uint8_t IO_ERROR_RESET[] = "Reset after SDcard I/O error";

uint8_t cfgSaveBytecode() {
  uint32_t proj_size = (*((uint32_t *)POJECT_ADDR) & 0xFFFFFF) + 4;
  if (proj_size < 16) return 0; 
  if (proj_size > (POJECT_END - POJECT_ADDR)) return 0;
  return sdWriteFile("mainproj.mpr", (void *)POJECT_ADDR, proj_size);
}

void cfgCheckResetStatus() {
  if (memcmp((void *)RESET_MESSAGE, IO_ERROR_RESET, sizeof(IO_ERROR_RESET)) == 0) {
    cfg_reset_cnt = RESET_CFG_CNT;
    io_reset_cnt = RESET_SDIO_CNT;
  } else {
    cfg_reset_cnt = 0;
    io_reset_cnt = 0;
  }
  memset((void *)RESET_DATA, 0, RESET_DATA_LEN);
}

void cfgResetDevice(uint8_t resetCode) {
  memcpy((void *)RESET_MESSAGE, IO_ERROR_RESET, sizeof(IO_ERROR_RESET));
  switch (resetCode) {
  case IO_RESET:
    io_reset_cnt++;
    break;
  case CFG_RESET:
    cfg_reset_cnt++;
    break;
  }
  RESET_CFG_CNT = cfg_reset_cnt;
  RESET_SDIO_CNT = io_reset_cnt;
  HAL_NVIC_SystemReset();
}

void prfPrepareDirs() {
  char dirname[] = "PRF_1";
  for (uint8_t i = 1; i <= 5 ; i++) {
    dirname[4] = 0x30 + i;
    if (f_stat(dirname, 0) != FR_OK) {
      f_mkdir(dirname);
    }
  }  
}

uint8_t cfgRead_attempt() {
  if (!sdOk()) return 0;
  prfPrepareDirs();
  sdSetCurDir("");
  if (!sdOpenForRead("k928.cfg")) return 0;
  uint8_t ok = 0;
  do {
    if (f_size(&SDFile) != sizeof(cfg_profile) + 1 + 2) break;
    if (!sdRead((void *)cfg_profile, sizeof(cfg_profile))) break;
    if (!sdRead(&cfg_cur_profile, 1)) break;
    ok = (cfg_cur_profile - 1) <= 4;
  } while(0);
  return sdClose() && ok;
}

void cfgRead() {
  if (!cfgRead_attempt()) {
    if (cfg_reset_cnt < MAX_CFG_READ_ATTEMPTS) {
      cfgResetDevice(CFG_RESET);
    } else {
      cfgDefault();
      cfgSave();
    }
  }
  cfg_reset_cnt = 0;
  cfgApply();
  cfgSetProjectDir();
}

void cfgSave() {
  if (!sdOk()) return;
  sdSetCurDir("");
  if (!sdOpenForWrite("k928.cfg")) return;
  do {
    if (!sdWrite((void *)cfg_profile, sizeof(cfg_profile))) break;
    if (!sdWrite(&cfg_cur_profile, 1)) break;
  } while(0);
  sdClose();
}

void cfgDefault() {
  TProfile *prf;
  for(uint8_t i = 0; i < 5; i++) {
    prf = &cfg_profile[i];
    prf->ledLight = 100;
    prf->ScreenSaverTime = 2;
    prf->buttonSound = 0;
    prf->plcAddress = 1;
    prf->bitRate = 4;
    prf->parityCheck = 1;
  }
  cfg_cur_profile = 1;
}

void cfgSetProjectDir() {
  char dirname[] = "PRF_?";
  dirname[4] = 0x30 + cfg_cur_profile;
  sdSetCurDir(dirname);
}

void cfgApply() {
  TProfile *prf = &cfg_profile[cfg_cur_profile - 1];
  kbd_sound_enabled = prf->buttonSound;
}
