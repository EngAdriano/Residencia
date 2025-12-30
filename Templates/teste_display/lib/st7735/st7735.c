#include "st7735.h"
#include "board_display.h"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

/* ===============================
   Comandos ST7735
   =============================== */
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A

#define MADCTL_MX  0x40
#define MADCTL_MY  0x80
#define MADCTL_RGB 0x00

/* ===============================
   GPIO helpers
   =============================== */
#define CS_HIGH()  gpio_put(BOARD_LCD_CS, 1)
#define CS_LOW()   gpio_put(BOARD_LCD_CS, 0)
#define DC_CMD()   gpio_put(BOARD_LCD_DC, 0)
#define DC_DATA()  gpio_put(BOARD_LCD_DC, 1)
#define RST_LOW()  gpio_put(BOARD_LCD_RST, 0)
#define RST_HIGH() gpio_put(BOARD_LCD_RST, 1)

/* ===============================
   Funções internas
   =============================== */
static inline void spi_write_u8(uint8_t v)
{
    spi_write_blocking(board_display_get_spi(), &v, 1);
}

static void write_cmd(uint8_t cmd)
{
    DC_CMD();
    CS_LOW();
    spi_write_u8(cmd);
    CS_HIGH();
}

static void write_data(const uint8_t *data, size_t len)
{
    DC_DATA();
    CS_LOW();
    spi_write_blocking(board_display_get_spi(), data, len);
    CS_HIGH();
}

static void set_window(uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    write_cmd(ST7735_CASET);
    data[0] = 0;
    data[1] = x0;
    data[2] = 0;
    data[3] = x1;
    write_data(data, 4);

    write_cmd(ST7735_RASET);
    data[1] = y0;
    data[3] = y1;
    write_data(data, 4);

    write_cmd(ST7735_RAMWR);
}

/* ===============================
   API pública
   =============================== */
void ST7735_Init(void)
{
    /* Reset físico */
    RST_LOW();
    sleep_ms(20);
    RST_HIGH();
    sleep_ms(150);

    write_cmd(ST7735_SWRESET);
    sleep_ms(150);

    write_cmd(ST7735_SLPOUT);
    sleep_ms(150);

    write_cmd(ST7735_COLMOD);
    uint8_t color = 0x05; // RGB565
    write_data(&color, 1);

    ST7735_SetRotation(1);

    write_cmd(ST7735_DISPON);
    sleep_ms(100);

    ST7735_FillScreen(ST7735_BLACK);
}

void ST7735_SetRotation(uint8_t rotation)
{
    uint8_t madctl = MADCTL_RGB;

    switch (rotation & 3)
    {
        case 0: madctl |= MADCTL_MX | MADCTL_MY; break;
        case 1: madctl |= MADCTL_MY; break;
        case 2: break;
        case 3: madctl |= MADCTL_MX; break;
    }

    write_cmd(ST7735_MADCTL);
    write_data(&madctl, 1);
}

void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= ST7735_WIDTH || y >= ST7735_HEIGHT)
        return;

    set_window(x, y, x, y);

    uint8_t data[2] = { color >> 8, color & 0xFF };
    write_data(data, 2);
}

void ST7735_FillScreen(uint16_t color)
{
    set_window(0, 0, ST7735_WIDTH - 1, ST7735_HEIGHT - 1);

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    DC_DATA();
    CS_LOW();

    for (uint32_t i = 0; i < ST7735_WIDTH * ST7735_HEIGHT; i++)
    {
        uint8_t px[2] = { hi, lo };
        spi_write_blocking(board_display_get_spi(), px, 2);
    }

    CS_HIGH();
}

void ST7735_DrawString(uint16_t x, uint16_t y,
                       const char *str,
                       FontDef font,
                       uint16_t color,
                       uint16_t bgcolor)
{
    while (*str)
    {
        char c = *str++;

        if (c < 32 || c > 126)
            continue;

        set_window(x, y,
                   x + font.width - 1,
                   y + font.height - 1);

        DC_DATA();
        CS_LOW();

        for (uint8_t row = 0; row < font.height; row++)
        {
            uint16_t line =
                font.data[(c - 32) * font.height + row];

            for (uint8_t col = 0; col < font.width; col++)
            {
                uint16_t px =
                    (line & (1 << (font.width - 1 - col)))
                        ? color
                        : bgcolor;

                uint8_t d[2] = { px >> 8, px & 0xFF };
                spi_write_blocking(board_display_get_spi(), d, 2);
            }
        }

        CS_HIGH();
        x += font.width;
    }
}

