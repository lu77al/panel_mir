#ifndef __AL_KEYBOARD_H__
#define __AL_KEYBOARD_H__
#include "stm32f7xx_hal.h"

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
