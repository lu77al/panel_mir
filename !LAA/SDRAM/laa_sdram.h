#ifndef __AL_SDRAM_H__
#define __AL_SDRAM_H__

#include "stm32f7xx_hal.h"

// Magic settings from http://narodstream.ru/stm-urok-62-fmc-sdram-chast-2/
#define SDRAM_TIMEOUT                            ((uint32_t)0xFFFF)
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)
#define REFRESH_COUNT ((uint32_t)0x0603)   // SDRAM refresh counter (100Mhz SD clock)

#define SDRAM_BANK_ADDR                         ((uint32_t)0xC0000000) // Phisical adress of SDRAM start

// --- My memory map of SDRAM -------------------------------------------------------------------------------------------
//         NAME           Value (Address)                     Purpose                          Size
#define POJECT_ADDR     SDRAM_BANK_ADDR                 // Project to execute           0x2EB800 = 3061760
#define POJECT_END      SD_BUFFER
#define SD_BUFFER       (SDRAM_BANK_ADDR + 0x2EB800)    // SD read / write buffer       0x4000 = 16384   
#define VARS_INTEGER    (SDRAM_BANK_ADDR + 0x2EF800)    // Internal integer vars        0x9C50 = 40016
#define VARS_REAL       (SDRAM_BANK_ADDR + 0x2F9450)    // Internal real vars           0x9C50 = 40016
#define TFT_CACHE       (SDRAM_BANK_ADDR + 0x3030A0)    // Heap with fonts, bmps, ...   0x385F60 = 3694432
#define TFT_CACHE_END   TFT_SCREEN
#define TFT_SCREEN      (SDRAM_BANK_ADDR + 0x689000)    // TFT video memory double buffered 
#define SDRAM_END       (SDRAM_BANK_ADDR + 0x800000)    // Accessing this address causes hard fault

extern uint8_t  *sd_buf;  //TODO move it to sdcard

void sdramInit();

#endif // __AL_SDRAM_H__
