/******************************************************************************
 *  Components for setup menus
 ******************************************************************************/
#include "laa_components.h"
#include "stdlib.h"
#include "string.h"
#include "laa_scr_tasks.h"
#include "laa_interface.h"
#include "laa_keyboard.h"
#include "laa_tft_lib.h"
#include "laa_utils.h"

#define SYS_BG      0x000088
#define ITEM_FG     0xffffff
#define SEL_BG      0xffff00
#define SEL_FG      0x0000ff
#define ITEM_FNT    SF_12x24
#define ITEM_FNT_W  12
#define ITEM_FNT_H  24
#define ITEM_STEP   28
#define LINES_CNT   7
#define X0          10
#define Y0          38
#define X_VALUE     280
#define HEAD_FNT    SF_16x32
#define HEAD_FNT_W  16
#define HEAD_FNT_H  32
#define HEAD_FG     0x88ff88

/* There is an idea ...
 *  - don't use single buffered mode
 *  - instead of this use some simplified string pool to implement logpad
 */

void cmpInitMenu(TListMenu *mnu) {
  mnu->state = malloc(sizeof(TListMenuState));
  uint8_t cnt = 0;
  const TMenuItem *item = mnu->item;
  while (item->text) {
    item++;
    cnt++;
  }
  mnu->state->count = cnt;
  mnu->state->current = 0;
  mnu->state->first = 0;
}

void cmpDrawMenu(TListMenu *mnu) {
  if (!mnu->state) cmpInitMenu(mnu); // Check init
  TListMenuState *state = mnu->state;
  scrResetPoint(0);  // Start drawing
  scrGoDouble();
  scrCLS(SYS_BG);     // Clear screen
  scrSetFG(HEAD_FG);    // Header
  scrSetFontStatic(HEAD_FNT);
  scrSetTextPos((TFT_WIDTH - HEAD_FNT_W * strlen(mnu->header)) / 2, 0);
  scrTextOutStatic(mnu->header);
  scrSetFontStatic(ITEM_FNT); // Status / hint
  scrSetTextPos(ITEM_FNT_W / 2, TFT_HEIGHT - ITEM_FNT_H);
  scrTextOutStatic(mnu->status);
  uint16_t y = Y0;      // Prepare to draw items
  uint8_t limit = state->first + LINES_CNT;
  if (limit > state->count) limit = state->count; 
  for (uint8_t i = state->first ; i < limit ; i++) { // Items loop
    if (i == state->current) {  // Draw selection and set text color
      scrSetBG(SEL_BG);
      scrBar(X0, y, mnu->selection_width, ITEM_STEP);
      scrSetFG(SEL_FG);
    } else {
      scrSetFG(ITEM_FG);
    }
    char num[4];        // Print index
    sprintf(num, "%hu", i + 1);
    scrSetTextPos(X0 + ITEM_FNT_W / 2, y);
    scrTextOut(num);
    
    uint8_t x = X0 + ITEM_FNT_W / 2 + ITEM_FNT_W * (strlen(num) + 1); // Print main text
    scrSetTextPos(x, y);
    scrTextOutStatic(mnu->item[i].text);
    if (mnu->item[i].value) { // Print value
      const TMenuValue *value = mnu->item[i].value;
     scrSetTextPos(X_VALUE, y);
      if (value->text) {
        const char *text = value->text;
        for (uint8_t i = value->min; i < *(value->value); i++) {
          text += strlen(text) + 1;
        }
        scrTextOutStatic(text);
      } else {
        sprintf(num, "%hu", *(value->value));
        scrTextOut(num);
      }
    }
    y += ITEM_STEP;
  }
  if (state->first != 0) {  // Top scroll line
    scrSetFG(ITEM_FG);
    scrSetLinePattern(0x0F0F0F0F);
    scrSetLineWidth(2);
    scrLine(X0, Y0 - 2, X0 + mnu->selection_width, Y0 - 2);
  }
  if (state->count - state->first > LINES_CNT) {  // Bottom scroll line
    scrSetFG(ITEM_FG);
    scrSetLinePattern(0x0F0F0F0F);
    scrSetLineWidth(2);
    scrLine(X0, Y0 + LINES_CNT * ITEM_STEP + 2,
            X0 + mnu->selection_width,  Y0 + LINES_CNT * ITEM_STEP + 2);
  }
}

void cmpMenuUserInput(TListMenu *mnu, uint8_t key) {
  if (!mnu->state) cmpInitMenu(mnu); // Check init
  TListMenuState *state = mnu->state;
  const TMenuItem *item = &mnu->item[state->current];
  TMenuValue *value = item->value;
  switch (key) {
  case KEY_UP:
    if (state->current == 0) break;
    state->current--;
    if (state->first > state->current) {
      state->first = state->current;
    }
    uiNeedUpdate = 1;
    break;
  case KEY_DOWN:
    if (state->current >= state->count - 1) break;
    state->current++;
    if (state->current - state->first >= LINES_CNT) {
      state->first = state->current - LINES_CNT + 1;
    }
    uiNeedUpdate = 1;
    break;
  case KEY_ENTER: {
    if (item->onEnter) item->onEnter();
    break; }
  case KEY_BACK:
    if (mnu->onExit) mnu->onExit();
    break;
  case KEY_LEFT:
    if (!value) break;
    if (*(value->value) <= value->min) {
      *(value->value) = value->max;
    } else {
      (*(value->value))--;
    }
    uiNeedUpdate = 1;
    break;
  case KEY_RIGHT:
    if (!value) break;
    if (*(value->value) >= value->max) {
      *(value->value) = value->min;
    } else {
      (*(value->value))++;
    }
    uiNeedUpdate = 1;
    break;
  }
}

#define MEMO_BUF_SIZE  127
static uint16_t memo_col;
static uint16_t memo_row;
static uint32_t memo_color;
static uint8_t  memo_pinter_index;
static uint8_t  memo_ready;
static const char *memo_header;
static const char *memo_status;
static char     log_data[MEMO_BUF_SIZE + 1];
static uint8_t  log_len;

#define MEMO_X0  (ITEM_FNT_W / 2)
#define MEMO_Y0  (HEAD_FNT_H + ITEM_FNT_H / 2)
#define MEMO_W   (TFT_WIDTH / ITEM_FNT_W - 1)
#define MEMO_H   ((TFT_HEIGHT - HEAD_FNT_H) / ITEM_FNT_H - 2)

void cmpLogMemoDraw();
void cmpLogMemoHidePointer();

void cmpLogMemoInit(const char *header, const char *status) {
  memo_header = header;
  memo_status = status;
  memo_ready = 0;
  memo_col = 0;
  memo_row = 0;
  memo_pinter_index = 0;
  log_len = 0;
  uiNeedUpdate = 1;
  log_data[MEMO_BUF_SIZE] = 0;
  memo_color = 0xffffff;
  uiDrawScreenRoutine = cmpLogMemoDraw;
}

void cmpLogMemoPrint(const char *text) {
  int16_t text_len = strlen(text);
  if (text_len > MEMO_BUF_SIZE - log_len) {
    text_len = MEMO_BUF_SIZE - log_len;
  }
  if (text_len <= 0) return;
  memmove(&log_data[log_len], text, text_len);
  log_len += text_len;
  uiNeedUpdate = 1;
}

void cmpLogMemoPrintColor(const char *text, uint32_t color) {
  log_data[log_len++] = 0;
  log_data[log_len++] = 0;
  laaSet24(&log_data[log_len], color);
  log_len += 3;
  cmpLogMemoPrint(text);
}

void cmpLogMemoForceUpdate() {
  if (uiDrawScreenRoutine == cmpLogMemoDraw) {
    uiNeedUpdate = 1;
  }
}

void cmpLogMemoNextLine() {
  log_data[log_len++] = 0;
  log_data[log_len++] = 1;
  uiNeedUpdate = 1;
}  

void cmpLogMemoInitDraw() {
  memo_ready = 1;
  scrGoSingle();
  scrCLS(SYS_BG);       // Clear screen
  scrSetBG(SYS_BG);
  scrSetFG(HEAD_FG);    // Header
  scrSetFontStatic(HEAD_FNT);
  scrSetTextTransparency(0);
  scrSetTextPos((TFT_WIDTH - HEAD_FNT_W * strlen(memo_header)) / 2, 0);
  scrTextOutStatic(memo_header);
  scrSetFontStatic(ITEM_FNT); // Status / hint
  scrSetTextPos(ITEM_FNT_W / 2, TFT_HEIGHT - ITEM_FNT_H);
  scrTextOutStatic(memo_status);
}

static const char pointer[] = "-\\|/"; 

void cmpLogMemoShowPointer() {
  scrSetFG(ITEM_FG);
  scrSetTextPos(MEMO_X0 + memo_col * ITEM_FNT_W, MEMO_Y0 + memo_row * ITEM_FNT_H);
  scrTextOutLen(&pointer[memo_pinter_index], 1);
  memo_pinter_index++;
  memo_pinter_index &= 3;
}

void cmpLogMemoHidePointer() {
  scrSetTextPos(MEMO_X0 + memo_col * ITEM_FNT_W, MEMO_Y0 + memo_row * ITEM_FNT_H);
  scrTextOutLen(" ", 1);
}


void cmpLogMemoNextLine_() {
  memo_col = 0;
  if (memo_row < MEMO_H - 1) {
    memo_row++;
  } else {
    scrCopyRect(MEMO_X0, MEMO_Y0 + ITEM_FNT_H,
                MEMO_W * ITEM_FNT_W, (MEMO_H - 1) * ITEM_FNT_H,
                MEMO_X0, MEMO_Y0);
    scrBar(MEMO_X0, MEMO_Y0 + (MEMO_H - 1) * ITEM_FNT_H,
           MEMO_W * ITEM_FNT_W, ITEM_FNT_H);
  }
}

void cmpLogMemoPrintString(char *ch, uint8_t len) {
  while (len) {
    uint8_t print_len = MEMO_W - memo_col;
    if (print_len > len) print_len = len;
    scrSetFG(memo_color);
    scrSetTextPos(MEMO_X0 + memo_col * ITEM_FNT_W, MEMO_Y0 + memo_row * ITEM_FNT_H);
    scrTextOutLen(ch, print_len);
    ch += print_len;
    len -= print_len;
    memo_col += print_len;
    if (memo_col >= MEMO_W) {
      cmpLogMemoNextLine_();
    }
  }
}

void cmpLogMemoDraw() {
  scrResetPoint(0);     // Start drawing
  if (!memo_ready) cmpLogMemoInitDraw();
  if (log_len == 0) {
    cmpLogMemoShowPointer();
    return;
  }
  char *ch = log_data;
  while (log_len) {
    if (*ch) {
      uint8_t len = strlen(ch);
      if (len > log_len) len = log_len;
      cmpLogMemoPrintString(ch, len);
      log_len -= len;
      ch += len;
    } else {
      uint8_t action = ch[1];
      ch += 2;
      log_len -= 2;
      if (action == 0) {
        memo_color = laaGet24(ch);
        ch += 3;
        log_len -= 3;
      } else if (action == 1) {
        cmpLogMemoHidePointer();
        cmpLogMemoNextLine_();
      }
    }
  }
}
