#ifndef __AL_CONFIG_H__
#define __AL_CONFIG_H__

#include "stm32f7xx_hal.h"

typedef struct TProfile {
  uint8_t ledLight;
  uint8_t ScreenSaverTime;
  uint8_t buttonSound;
  uint8_t plcAddress;
  uint8_t bitRate;
  uint8_t parityCheck;
} TProfile;

extern uint8_t   cfg_cur_profile;
extern TProfile  cfg_profile[5];

void cfgRead();
void cfgSave();
void cfgCheckResetStatus();
void cfgSetProjectDir();
void cfgApply();
void cfgDefault();
void cfgSetProjectDir();

#endif // __AL_CONFIG_H__

