/******************************************************************************
 *  Handling SDCARD
 *   - init
 *   - operations with temporary buffer
 *   - override lib functions to work with specified directory (may be it's a "duct tape")
 ******************************************************************************/
#include "laa_sdcard.h"
#include "string.h"
#include "fatfs.h"
#include "laa_crc16.h"

extern char SDPath[4];   /* SD logical drive path */
extern FATFS SDFatFS;    /* File system object for SD logical drive */
extern FIL SDFile;       /* File object for SD */

static char     sd_curdir[64] = "";
static uint8_t  sd_ok = 0;
static uint16_t sd_file_crc16;
static uint32_t sd_file_size;
static uint32_t sd_file_pos;
static BYTE     sd_file_mode;
static uint32_t sd_read_error_cnt = 0;

#define SD_BUF_SIZE 2048
static uint8_t  sdbuf[SD_BUF_SIZE];

void sdMount() {
  sd_ok = (f_mount(&SDFatFS, SDPath, 1) == FR_OK);
  if (sd_ok) {
    strcpy(sd_curdir, SDPath);
  }  
}

uint8_t sdOk() {
  return sd_ok;
}  

void sdSetCurDir(char *dir) {
  strcpy(sd_curdir, SDPath);
  strcat(sd_curdir, dir);
  strcat(sd_curdir, "/");
}

uint8_t sdOpen(const char *name,  BYTE mode) {
  if (!sd_ok) return 0;
  char full_name[72];
  strcpy(full_name, sd_curdir);
  strcat(full_name, name);
  if (f_open(&SDFile, full_name, mode) != FR_OK) return 0;
  init_crc16((void *)&sd_file_crc16);
  if (mode == FA_READ) {
    sd_file_size = f_size(&SDFile);
    if (sd_file_size > 2) sd_file_size -= 2;
  } else {
    sd_file_size = 0;
  }
  sd_file_mode = mode;
  sd_file_pos = 0;
  return 1;
}

uint8_t sdOpenForRead(const char *name) {
  return sdOpen(name,  FA_READ);
}

uint8_t sdOpenForWrite(const char *name) {
  return sdOpen(name,  FA_CREATE_ALWAYS | FA_WRITE);
}

uint8_t sdClose() {
  uint8_t res = 0;
  if (sd_file_mode == FA_READ) {
    if (sd_file_pos <= sd_file_size) {
      sdRead(0, sd_file_size - sd_file_pos);
      uint16_t crc = 0;
      uint32_t byteread;
      if (f_read(&SDFile, &crc, 2, (void *)&byteread) == FR_OK) {
        res = (byteread == 2) && (crc == sd_file_crc16);
      }  
    }
  } else {
    uint32_t written;
    if (f_write(&SDFile, &sd_file_crc16, 2, (void *)&written) == FR_OK) {
      res = written == 2;
    }  
  }  
  f_close(&SDFile);
  return res;
}  

uint8_t sdRead(uint8_t *buffer, uint32_t size) {
  uint32_t byteread;
  uint32_t portion;
  while (size) {
    portion = size > SD_BUF_SIZE ? SD_BUF_SIZE : size;
    if (f_read(&SDFile, sdbuf, portion, (void *)&byteread) != FR_OK) return 0;
    if (byteread != portion) return 0;
    for (uint16_t i = 0; i < portion; i++) {
      add_byte_to_crc16((void *)&sd_file_crc16, sdbuf[i]);
    }
    if (buffer) {
      memcpy(buffer, sdbuf, portion);
      buffer += portion;
    }
    sd_file_pos += portion;
    size -= portion;
  }  
  return 1;
}

/*
uint8_t sdSeek(uint32_t pos) {
  return f_lseek(&SDFile, pos) == FR_OK;
}
*/

