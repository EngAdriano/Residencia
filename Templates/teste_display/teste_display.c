#include <stdio.h>
#include "pico/stdlib.h"

#include "board_display.h"
#include "st7735.h"
#include "fonts.h"

int main(void)
{
    stdio_init_all();
    sleep_ms(1000);

    ST7735_Init();
    ST7735_FillScreen(ST7735_BLACK);

    ST7735_DrawString(10, 10, "ENERGIA",
                    Font_11x18,
                    ST7735_GREEN,
                    ST7735_BLACK);

    ST7735_DrawString(10, 40, "123.45 V",
                    Font_7x10,
                    ST7735_WHITE,
                    ST7735_BLACK);

    while (true)
    {
        tight_loop_contents();
    }
}
