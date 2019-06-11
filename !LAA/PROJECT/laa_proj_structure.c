/******************************************************************************
 *  Code file structure
 ******************************************************************************/
#include "laa_proj_structure.h"
#include "laa_sdcard.h"
#include "laa_sdram.h"
#include "laa_utils.h"
#include "string.h"

void prjClear() {
  memcpy(PROJECT, (uint8_t[]){0x04, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00}, 8);
}

uint8_t *prjCRC() {
  return PROJECT + PROJECT_SIZE;
}

uint8_t prjSave() {
  uint32_t size = PROJECT_SIZE;
  if (size < 16) return 0; 
  if (size > (POJECT_END - POJECT_ADDR)) return 0;
  return sdWriteFile("mainproj.mpr", PROJECT, size);
}
