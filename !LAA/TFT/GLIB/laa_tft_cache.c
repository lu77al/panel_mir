/******************************************************************************
 *  Routines to manipulate graphical objects such as fonts and bmp's in
 *  the RAM and ROM. Search and select, locate new, delete ...
 ******************************************************************************/

#include "string.h"
#include "laa_global_utils.h"
#include "laa_tft_cache.h"
#include "laa_sdram.h"
#include "laa_tft_lib.h"

/* Cahche for graphical objects
 *  object structure:
 *    00 - ID (12 bytes) - Filename name(8) + delimiter(1) + extension(3) (if len<12 -> rest 0)
 *    12 - pointer to next object (4 bytes). 0 - last object
 *    16 - data
 *      --- font ---
 *       16 - width  (1 byte)
 *       17 - height (1 byte)
 *       18 - bytes per char (2 bytes)
 *       20 - font points: chars 0..255, top..bottom, left..right (byte alligned MSB first)
 */

uint8_t   tft_search_bouth = OFF;   
uint8_t   *tft_obj_first = 0; // First object in the queue (search starts here)
uint8_t   *tft_obj_last  = 0; // Last object in the queue (contains null pointer to the next object, which is changed when next object is added)
uint8_t   *tft_obj_new;       // Adress to place next new object (also used to calculate free cache space)

//    --case setFont (user)
//   find in RAM  -> return if found
//   find in ROM  -> return if found
//   load from SD -> locate in RAM and return
//   set defaut system font   
//    --case setFont (system)
//   find in ROM  -> return if found
//   find in RAM  -> return if found
//   load from SD -> locate in RAM and return
//   set defaut system font   

/* Finds object in the list started at addr by id. Returns pointer
 *  if not found returns 0
 */
uint8_t *tftFindObjectAt(const char *id, uint8_t *addr) {
  while (addr) {
    if (!strncmp((char *)addr, id, 12)) break;
    addr = *(uint8_t **)(addr + 12);
  }
  return addr;
}

/* If first char is '*' find in ROM then in RAM
 *  else find in RAM then in ROM
 *   - char '*' is not used in search
 */
uint8_t *tftFindObject(const char *id) {
  uint8_t *found;
  if (id[0] == '*') {
    id++;      // Find in ROM then in RAM without '*'
    found = tftFindObjectAt(id, (uint8_t *)TFT_ROM_CACHE);
    if (!found && tft_search_bouth) found = tftFindObjectAt(id, tft_obj_first);
  } else {      // find in RAM then in ROM
    found = tftFindObjectAt(id, tft_obj_first);
    if (!found && tft_search_bouth) found = tftFindObjectAt(id, (uint8_t *)TFT_ROM_CACHE);
  }
  return found;
}  

/* Returns size of empty space in the cache
 */
uint32_t tftGetFreeCache() {
  if (!tft_obj_first) return TFT_CACHE_END - TFT_CACHE; // [_____________]
  if (tft_obj_first < tft_obj_new) return TFT_CACHE_END - (uint32_t)tft_obj_new; // [...f****n____]
  return tft_obj_first - tft_obj_new; // [****n____f***...] "*" - used; "_" - free; "." - uused (leakage)
}  

/* Deletes objects to free chache
 */
void tftFreeCache() {
  if (!tft_obj_first) {
    //TODO error - too big object
  }
  if (tft_obj_first < tft_obj_new) { // if not enough space in the end of cache
    tft_obj_new = (uint8_t *)TFT_CACHE;  // try to replace tft_obj_new to the start
  } else { 
    tft_obj_first = *((uint8_t **)(tft_obj_first + 12)); // otherwise delete first object
  }
}  

/* Locates space in cache and forms header of a new object
 */
uint8_t *tftLocateCache(uint32_t size, const char* id) {
  size = (size + 16 + 3) & ~3; // +16 bytes (header = id + pntr) +fit size to 4 byte step
  while (size > tftGetFreeCache()) tftFreeCache();
  if (tft_obj_first) {
    *((uint8_t **)(tft_obj_last + 12)) = tft_obj_new;
  } else {
    tft_obj_first = tft_obj_new = tft_obj_last = (uint8_t *)TFT_CACHE;
  }
  tft_obj_last = tft_obj_new;
  tft_obj_new += size;
  *((uint8_t **)(tft_obj_last + 12)) = 0;
  strncpy((void *)tft_obj_last, id, 12);
  return tft_obj_last;
}

/* Free all the cache
 */
void tftClearCache() {
  tft_obj_first = 0;
}
