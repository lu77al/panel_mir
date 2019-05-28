#ifndef __AL_KEYBOARD_H__
#define __AL_KEYBOARD_H__
#include "stm32f7xx_hal.h"

/*
const uint8_t kbd_code[KBD_SIZE] = { // KeyCodes
  0x37, 0x38, 0x39,
  0x34, 0x35, 0x36,
  0x31, 0x32, 0x33,
  0x30, 0x2E, 0x2D,
  0x06, 0x7F, 0x13,
  0x58, 0x71, 0x3F,
  0x70, 0x72, 0x73
};  
*/

#define KEY_UP      0x71
#define KEY_DOWN    0x72
#define KEY_LEFT    0x70
#define KEY_RIGHT   0x73
#define KEY_ENTER   0x13
#define KEY_BACK    0x7F

void kbdInit(); // Init structures
void kbdClear(); // Clear queues
uint8_t kbdIsKeyPressed(uint8_t key); // Check key state
void kbdScan(); // Force next keyboard scan
// Keycodes (press and repeat)
uint8_t kbdPollKey();    // Get next keycode
uint8_t kbdPeekKey();    // Get next keycode (but leave it in the queue)
void kbdAddKey(uint8_t key); // Add keycode to queue
// Events (presed(+) / released(-))
int16_t kbdPollEvent();  // Get next event
int16_t kbdPeekEvent();  // Get next event (but leave it in the queue)
void kbdAddEvent(int16_t event); // Add event to queue

#endif // __AL_KEYBOARD_H__
