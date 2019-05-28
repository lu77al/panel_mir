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
   
#define  KBD_SIZE   21  // Key count
#define  KBD_LEN    32  // Queues lengths (keys and events)
#define  KBD_HEIGHT 5   // Count of scan lines (energizing)
#define  KBD_WIDTH  5   // Count of scan columns (reading)

#define KBD_PRESS_THH   3
#define KBD_HYST        3
#define KBD_REPEAT_THH  80
#define KBD_RESTART     50

uint8_t  kbd_key[KBD_SIZE + 1]; // Keys state counters
uint8_t  kbd_input[KBD_LEN];    // key input queue buffer
uint8_t  kbd_input_front = 0;
uint8_t  kbd_input_cnt = 0;
uint16_t kbd_event[KBD_LEN];    // key event queue buffer
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
uint8_t kbd_sound_time = 0;
uint8_t kbd_sound_on = 0;

extern TIM_HandleTypeDef htim12;

/* Start / correct stop of buzzer PWM
 */
void kbd_buzzer() {
  if (kbd_sound_time) {
    if (!kbd_sound_on) {
      HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);
      kbd_sound_on = 1;
    }  
    kbd_sound_time--; 
  } else {
    if (kbd_sound_on) {
      HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_1);
      kbd_sound_on = 0;
    } 
  }  
}

/* Scanning keyboard line by line (schematic specific procedure)
 *  one line per call
 *  also drive buzzer
 */
void kbdScan() {
  uint8_t input = ~(GPIOF->IDR >> 6); // PF10 .. PF6
  for (uint8_t col = 0; col < KBD_WIDTH; col++, input >>= 1) {
    uint8_t scn = kbd_scancode[kbd_line][col];
    if (scn >= KBD_SIZE) continue;
    uint8_t* key = &kbd_key[scn];
    if (input & 1) { // Contact detected
      (*key)++;
      if (*key == KBD_PRESS_THH) { // First press
        *key += KBD_HYST;
 //if (ui_state == UIS_TEST_KEYBOARD) kbd_sound_time = 12; // 2
//        kbd_sound_time = 8;
        kbdAddKey(kbd_code[scn]);
        kbdAddEvent((int16_t)kbd_code[scn]);
      } else if (*key == KBD_REPEAT_THH) { // Repeat pressed key
        *key = KBD_RESTART;
        kbdAddKey(kbd_code[scn]);
      }  
    } else { // No contact
      if (*key == 0) continue;
      if (*key > KBD_PRESS_THH + KBD_HYST) {
        *key = KBD_PRESS_THH + KBD_HYST;
      }
      (*key)--;
      if (*key == KBD_PRESS_THH) { // Key released
        *key = 0;
        kbdAddEvent(-(int16_t)kbd_code[scn]);
      }
    }
  }  
  laaWritePin(kbd_line_pin[kbd_line], 1);
  kbd_line++;
  kbd_line %= KBD_HEIGHT;
  laaWritePin(kbd_line_pin[kbd_line], 0);
  kbd_buzzer();
}  

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
  kbd_input_cnt--;
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
  kbd_event_cnt--;
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
