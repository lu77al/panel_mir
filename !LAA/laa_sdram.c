/******************************************************************************
 *  Connecting SDRAM MT48LC4M32B2 to comtroller
 *  copied from here http://narodstream.ru/stm-urok-62-fmc-sdram-chast-1/
 *  (see pdf copies to recall how it was tuned in CubeMX)
 ******************************************************************************/
#include "laa_sdram.h"
   
extern SDRAM_HandleTypeDef hsdram1;

uint8_t  *sd_buf = (uint8_t *)SD_BUFFER; //TODO move it to sdcard

FMC_SDRAM_CommandTypeDef command;
__IO HAL_StatusTypeDef hal_stat;

void sdram_init() {
  SDRAM_HandleTypeDef *hsdram = &hsdram1;
  __IO uint32_t tmpmrd = 0;
  command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;
  hal_stat = HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT);
  HAL_Delay(2); // ----
  command.CommandMode = FMC_SDRAM_CMD_PALL;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;
  hal_stat = HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT);
  asm("nop"); // ----
  command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 8;
  command.ModeRegisterDefinition = 0;
  hal_stat = HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT);
  asm("nop"); // ----
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_2           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = tmpmrd;
  hal_stat = HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT);  
  hal_stat = HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT);
  HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT);  
}
