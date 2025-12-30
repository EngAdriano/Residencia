#ifndef BOARD_DISPLAY_H
#define BOARD_DISPLAY_H

#include "hardware/spi.h"

/* Ajuste conforme sua placa */
#define BOARD_DISPLAY_SPI spi0
#define BOARD_DISPLAY_BAUD (4000 * 1000)

/* Pinos do TFT */
#define BOARD_LCD_SCK   18
#define BOARD_LCD_MOSI  19
#define BOARD_LCD_CS    17
#define BOARD_LCD_DC    20
#define BOARD_LCD_RST   9

void board_display_init(void);
spi_inst_t *board_display_get_spi(void);

#endif
