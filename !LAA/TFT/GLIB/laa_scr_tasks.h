#ifndef __AL_SCR_TASKS_H__
#define __AL_SCR_TASKS_H__
#include "stm32f7xx_hal.h"

extern const char SF_08x12[];
extern const char SF_08x16[];
extern const char SF_12x24[];
extern const char SF_16x32[];
extern const char SF_24x48[];

//--- Rendering tasks management ---
void scrPerformNextTask(); // Render scr_task -> TFT
uint8_t scrIsRenderComplete(); // Ready to get new tasks
void scrSavePoint(uint8_t level);  // Save scr_task pointer for further redraw from this point (level = 1..15)
void scrResetPoint(uint8_t level); // Reset saved scr_task pointer to start drawing there (level = 0..15) 0 - from start
void scrStartRender(); // Start rendering process

//--- Modes, colors, settings ---
void scrGoDouble();  // Double buffered mode
void scrGoSingle();  // Single buffered mode
void scrSetBG(uint32_t color); // Set background color
void scrSetFG(uint32_t color); // Set foreground color
void scrSetLineWidth(int8_t width);  // Set line width
void scrSetLinePattern(int32_t pattern); // Set line pattern
void scrSetTextTransparency(int8_t transparent); // Set font transparency
void scrSetFont(const char *name); // Set font (save name in scr_task)
void scrEncodingOn(const char *name); // Enable encoding chars 
void scrEncodingOff(const char *name); // Disable encoding chars 
void scrSetFontStatic(const char *name); // Set font with static name (save pointer to name in scr_task)
void scrSetBMP(char *name, uint32_t trColor888);  // Set bmp (save name in scr_task)
void scrSetBMPstatic(char *name, uint32_t trColor888); // Set bmp with static name (save pointer to name in scr_task)

//--- Draw primitives ---
void scrCLS(int32_t color);     // Fill screen with color 
void scrBar(int16_t x, int16_t y, int16_t w, int16_t h); // Filled bar (BGColor)
void scrLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2); // Draw line
void scrMoveTo(int16_t x, int16_t y);  // Move line pointer to ...
void scrLineTo(int16_t x, int16_t y);  // Draw line to absolute
void scrLineRel(int16_t x, int16_t y); // Draw line to relative
void scrSetTextPos(int16_t x, int16_t y); // Set text cursor position
void scrTextOutLen(const char *text, uint8_t maxLen); // Print text dynamic (save text in scr_task)
void scrTextOutStaticLen(const char *text, uint8_t maxLen); // Print text static (save pointer to test in scr_task)
void scrTextOut(const char *text); // Print text dynamic (save text in scr_task)
void scrTextOutStatic(const char *text); // Print text static (save pointer to test in scr_task)
void scrInitPoly(uint8_t filled, uint8_t closed); // Start drawing polygon
void scrPolyVertex(int16_t x, int16_t y); // Add polygon vertex
void scrEllipse(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t s, uint16_t e, uint8_t filled, uint8_t closed); // Ellipse
void scrDrawBMP(int16_t x, int16_t y, uint8_t alpha); // Draw selected bmp with alpha

#endif // __AL_SCR_TASKS_H__

