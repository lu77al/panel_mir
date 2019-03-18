/******************************************************************************
 *  Keyboard driver and routines
 *    - Scanning
 *    - Current keyboard state   
 *    - Queue of key_codes
 *    - Queue of events (press/release)
 ******************************************************************************/
#include "laa_keyboard.h"
#include "string.h"
#include "laa_utils.h"
   
#define  KBD_SIZE   21 // Key count
#define  KBD_LEN    32 // Queues lengths (keys and events)
#define  KBD_HEIGHT 5  // Count of scan lines (energizing)
#define  KBD_WIDTH  5  // Count of scan columns (reading)

#define KBD_PRESS_THH   3
#define KBD_HYST        3
#define KBD_REPEAT_THH  50
#define KBD_RESTART     25

uint8_t  kbd_key[KBD_SIZE + 1]; // Keys state counters
uint8_t  kbd_input[KBD_LEN];   // key input queue buffer
uint8_t  kbd_input_front = 0;
uint8_t  kbd_input_cnt = 0;
uint16_t kbd_event[KBD_LEN];  // key event queue buffer
uint8_t  kbd_event_front = 0;
uint8_t  kbd_event_cnt = 0;

uint8_t kbd_index[255]; // KeyCode -> index
const uint8_t kbd_code[KBD_SIZE] = { // KeyCodes
  0x37, 0x38, 0x39,
  0x34, 0x35, 0x36,
  0x31, 0x32, 0x33,
  0x30, 0x2E, 0x2D,
  0x06, 0x7F, 0x13,
  0x58, 0x71, 0x3F,
  0x70, 0x72, 0x73
};  

const uint8_t kbd_name[KBD_SIZE][6] = {
  "7",    "8",    "9",      //  0   1   2    
  "4",    "5",    "6",      //  3   4   5    
  "1",    "2",    "3",      //  6   7   8
  "0",    ",",   "+/-",     //  9  10  11
 "Menu", "Back", "Enter"    // 12  13  14
  "F",    "Up",  "Set",     // 15  16  17
 "Left", "Down", "Right"    // 18  19  20
};    

const LAA_GPIO kbd_line_pin[KBD_HEIGHT] = {
  {GPIOB, GPIO_PIN_8},
  {GPIOB, GPIO_PIN_9},
  {GPIOB, GPIO_PIN_14},
  {GPIOB, GPIO_PIN_15},
  {GPIOA, GPIO_PIN_8}
};

const uint8_t kbd_scancode[KBD_HEIGHT][KBD_WIDTH] = {
  {20, 15,  2,  1,  0},   // PB8:   Right, F, 9, 8, 7
  {21, 16,  5,  4,  3},   // PB9:   -/-,  Up, 6, 5, 4
  {21, 17,  8,  7,  6},   // PB14:  -/-, SET, 3, 2, 1
  {21, 18, 11, 10,  9},   // PB15:  -/-, Left, +/-, ",", 0
  {21, 19, 14, 13, 12}    // PA8:   -/-, Down, Enter, "<=", Menu
};

uint8_t kbd_line = 0;

/* Scanning keyboard line by line (schematic specific procedure)
 */
void kbdScan() {
  uint8_t input = ~(GPIOF->IDR >> 6); // PF10 .. PF6
  for (uint8_t col = 0; col < KBD_WIDTH; col++, input >>= 1) {
    uint8_t scn = kbd_scancode[kbd_line][col];
    if (scn >= KBD_SIZE) continue;
    uint8_t* key = &kbd_key[scn];
    if (input & 1) {
      *key++;
      if (*key == KBD_PRESS_THH) {
        *key += KBD_HYST;
        kbdAddKey(kbd_code[scn]);
        kbdAddEvent((int16_t)kbd_code[scn]);
      } else if (*key == KBD_REPEAT_THH) {
        *key = KBD_RESTART;
        kbdAddKey(kbd_code[scn]);
      }  
    } else {
      
    }
  }  
  laaWritePin(kbd_line_pin[kbd_line], 1);
  kbd_line++;
  kbd_line %= KBD_HEIGHT;
  laaWritePin(kbd_line_pin[kbd_line], 0);
}  

/*
    uint8_t keycode = kbd_keycode[scancode];
    KbrdType *but = &kbd_but[keycode];
    but->released = 0;
    but->pressed = 0;
    if (in_pins & 1) but->rel_time = 0; else {
      if (but->rel_time < 5) but->rel_time++;
    }
    if (but->rel_time < 5) {
      but->state++;
      if (but->state == 1) {
//        if (!but->pressed) kbd_new_event = 1;
        kbd_new_event = 1;
        but->repeated = 1;
        but->pressed  = 1;
        kbd_pushkey(keycode);
        if (prf_sound) kbd_sound_time = 7; // 2
        if (ui_state == UIS_TEST_KEYBOARD) kbd_sound_time = 12; // 2
//        kbd_kbd_kbd = 2;
//        just_var = 15;
        kbd_last_scanned = scancode;  
      }  
      if (but->state > KBD_FIRST_DELAY) {
        but->repeated = 1;
        if (keycode != KEYCODE_SHIFT) kbd_pushkey(keycode); //  | 0x80
        but->state = KBD_FIRST_DELAY - KBD_NEXT_DELAY + 1;
        if (but->repcnt != 0xFF) but->repcnt++;
      }
    } else {
      if (but->state) {
//        if (!but->released) kbd_new_event = 1
        kbd_new_event = 1;
        but->released = 1;
      }  
      but->state = but->repcnt = 0;
    }  
    in_pins >>= 1;
    scancode++;
  }
  kbd_scan_group = (kbd_scan_group + 1) % 5;
  for (uint8_t i=0; i<5; i++) {
    HAL_GPIO_WritePin(kbd_line[i].Port, kbd_line[i].Pin, (GPIO_PinState)(i != kbd_scan_group));
  }  
  
}
*/

/* Init data structures
 */
void kbdInit() {
  memset(kbd_index, KBD_SIZE, 255);
  for (uint8_t i = 0; i < KBD_SIZE; i++) {
    kbd_index[kbd_code[i]] = i;
  }  
  memset(kbd_key, KBD_SIZE + 1, 0);
  kbdClear();
}  

/* Clear queues
 */
void kbdClear() {
  kbd_input_front = 0;
  kbd_input_cnt = 0;
  kbd_event_front = 0;
  kbd_event_cnt = 0;
}  

/*  Check key state
 */
uint8_t kbdIsKeyPressed(uint8_t key) {
  return (kbd_key[kbd_index[key]] >= KBD_PRESS_THH);
}  

/*  Get next keycode (but leave it in the queue)
 */
uint8_t kbdPeekKey() {
  if (kbd_input_cnt == 0) return 0;
  return kbd_input[kbd_input_front];
}  

/*  Get next keycode
 */
uint8_t kbdPollKey() {
  if (kbd_input_cnt == 0) return 0;
  uint8_t result = kbd_input[kbd_input_front];
  kbd_input_front++;
  kbd_input_front %= KBD_LEN;
  return result;
}  

/* Add key to queue
 */
void kbdAddKey(uint8_t key) {
  if (kbd_input_cnt >= KBD_LEN) return;
  kbd_input[(kbd_input_front + kbd_input_cnt) % KBD_LEN] = key;
  kbd_input_cnt++;
}  

/*  Get next event (but leave it in the queue)
 */
int16_t kbdPeekEvent() {
  if (kbd_event_cnt == 0) return 0;
  return kbd_event[kbd_event_front];
}  

/*  Get next event
 */
int16_t kbdPollEvent() {
  if (kbd_event_cnt == 0) return 0;
  int16_t result = kbd_event[kbd_event_front];
  kbd_event_front++;
  kbd_event_front %= KBD_LEN;
  return result;
}  

/* Add event to queue
 */
void kbdAddEvent(int16_t event) {
  if (kbd_input_cnt >= KBD_LEN) return;
  kbd_event[(kbd_event_front + kbd_event_cnt) % KBD_LEN] = event;
  kbd_event_cnt++;
}  

/*
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
*/