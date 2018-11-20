/******************************************************************************
 *  Handling SDCARD
 *   - init
 *   - operations with temporary buffer
 *   - override lib functions to work with specified directory (may be it's a "duct tape"
 ******************************************************************************/
#include "laa_sdcard.h"
#include "string.h"
#include "fatfs.h"

extern char SDPath[4];   /* SD logical drive path */
extern FATFS SDFatFS;    /* File system object for SD logical drive */
extern FIL SDFile;       /* File object for SD */

char sd_curdir[64] = "";

uint8_t sd_ok = 0;

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
  return (f_open(&SDFile, full_name, mode) == FR_OK);
}  

uint8_t sdOpenForRead(const char *name) {
  return sdOpen(name,  FA_READ);
}  

uint8_t sdOpenForWrite(const char *name) {
  return sdOpen(name,  FA_CREATE_ALWAYS | FA_WRITE);
}  

void sdClose() {
  f_close(&SDFile);
}  

uint8_t sdRead(uint8_t *buffer, uint32_t size) {
  uint32_t byteread;
  if (f_read(&SDFile, buffer, size, (void *)&byteread) != FR_OK) return 0;
  return (size == byteread);
}  

uint8_t sdSeek(uint32_t pos) {
  return f_lseek(&SDFile, pos) == FR_OK;
}  

