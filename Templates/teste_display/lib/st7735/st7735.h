#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>
#include <stdbool.h>
#include "fonts.h"

/* Display ST7735 1.8" */
#define ST7735_WIDTH   128
#define ST7735_HEIGHT  160

/* RGB565 */
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF

void ST7735_Init(void);
void ST7735_SetRotation(uint8_t rotation);
void ST7735_FillScreen(uint16_t color);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_DrawString(uint16_t x, uint16_t y,
                       const char *str,
                       FontDef font,
                       uint16_t color,
                       uint16_t bgcolor);

#endif
