#include "board_display.h"
#include "hardware/gpio.h"

void board_display_init(void)
{
    spi_init(BOARD_DISPLAY_SPI, BOARD_DISPLAY_BAUD);

    gpio_set_function(BOARD_LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(BOARD_LCD_MOSI, GPIO_FUNC_SPI);

    gpio_init(BOARD_LCD_CS);
    gpio_set_dir(BOARD_LCD_CS, GPIO_OUT);
    gpio_put(BOARD_LCD_CS, 1);

    gpio_init(BOARD_LCD_DC);
    gpio_set_dir(BOARD_LCD_DC, GPIO_OUT);
    gpio_put(BOARD_LCD_DC, 1);

    gpio_init(BOARD_LCD_RST);
    gpio_set_dir(BOARD_LCD_RST, GPIO_OUT);
    gpio_put(BOARD_LCD_RST, 1);
}

spi_inst_t *board_display_get_spi(void)
{
    return BOARD_DISPLAY_SPI;
}
