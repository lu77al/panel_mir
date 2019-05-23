/******************************************************************************
 *  Routines to manipulate graphical objects such as fonts and bmp's in
 *  the RAM and ROM. Search and select, locate new, delete ...
 ******************************************************************************/

#include "string.h"
#include "laa_utils.h"
#include "laa_tft_cache.h"
#include "laa_sdram.h"
#include "laa_tft_lib.h"

/* Cahche for graphical objects
 *  object structure:
 *    00 - ID (16 bytes) BMPtransparentHash(4) + Filename name(8) + delimiter(1) + extension(3)
 *    16 - pointer to next object (4 bytes). 0 - last object
 *    20 - data
 *      --- font ---
 *       0 - width  (1 byte)
 *       1 - height (1 byte)
 *       2 - bytes per char (2 bytes)
 *       4 - font points: chars 0..255, top..bottom, left..right (byte alligned MSB first)
 */

#define OBJ_ID_LENGHT   16
#define OBJ_NEXT_OFFS   16
#define OBJ_DATA_OFFS   20
#define OBJ_HEADER_SIZE 20

uint8_t   tft_search_bouth = ON;
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
    if (!strncasecmp((char *)addr, id, OBJ_ID_LENGHT)) break;
    addr = *(uint8_t **)(addr + OBJ_NEXT_OFFS);
  }
  return addr ? addr + OBJ_DATA_OFFS : 0;
}

/* If first char is '*' find in ROM then in RAM
 *  else find in RAM then in ROM
 *   - char '*' is not used in search
 */
uint8_t *tftFindObject(const char *id) {
  uint8_t *found;
  if (id[0] == '*') { // Find in ROM then in RAM
    id++;      // skip '*'
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
    tft_obj_first = *((uint8_t **)(tft_obj_first + OBJ_NEXT_OFFS)); // otherwise delete first object
  }
}  

/* Locates space in cache and forms header of a new object
 */
uint8_t *tftLocateCache(uint32_t size, const char* id) {
  size = (size + OBJ_HEADER_SIZE + 3) & ~3; // +OBJ_HEADER_SIZE bytes  +fit size to 4 byte step
  while (size > tftGetFreeCache()) tftFreeCache();
  if (tft_obj_first) {
    *((uint8_t **)(tft_obj_last + OBJ_NEXT_OFFS)) = tft_obj_new;
  } else {
    tft_obj_first = tft_obj_new = tft_obj_last = (uint8_t *)TFT_CACHE;
  }
  tft_obj_last = tft_obj_new;
  tft_obj_new += size;
  *((uint8_t **)(tft_obj_last + OBJ_NEXT_OFFS)) = 0;
  strncpy((void *)tft_obj_last, id, OBJ_ID_LENGHT);
  return tft_obj_last + OBJ_DATA_OFFS;
}

/* Free all the cache
 */
void tftClearCache() {
  tft_obj_first = 0;
}

/* Delete last located
 */
void tftDeleteLast() {
  if (!tft_obj_first) return;
  uint8_t *prev = 0;
  uint8_t *obj = tft_obj_first;
  while (1) {
    uint8_t *next = *((uint8_t **)(obj + OBJ_NEXT_OFFS));
    if (!next) break;
    prev = obj;
    obj = next;
    if (obj == tft_obj_last) break;
  }
  if (prev) {
    *((uint8_t **)(prev + OBJ_NEXT_OFFS)) = 0;
    tft_obj_last = prev;
    tft_obj_new = obj;
  } else {
    tft_obj_first = 0;
  }
}
