#include "ssd1306.h"
#include <string.h>

// Minimal 6x8 font (only ASCII 32..127) - for brevity we include a tiny subset
static const uint8_t font6x8[][6] = {
    // space (32)
    {0x00,0x00,0x00,0x00,0x00,0x00},
    // '!' (33)
    {0x00,0x00,0x5F,0x00,0x00,0x00},
    // '0'..'9' and letters are omitted for brevity; we'll implement simple numeric rendering below
};

static int i2c_write_command(i2c_inst_t *i2c, uint8_t addr, uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    int r = i2c_write_blocking(i2c, addr, buf, 2, false);
    return r;
}

void ssd1306_init(ssd1306_t *dev, i2c_inst_t *i2c, uint8_t addr, uint8_t width, uint8_t height) {
    dev->i2c = i2c;
    dev->address = addr;
    dev->width = width;
    dev->height = height;
    memset(dev->buffer, 0x00, sizeof(dev->buffer));

    // initialize sequence
    i2c_write_command(i2c, addr, 0xAE); // display off
    i2c_write_command(i2c, addr, 0x20); // memory mode
    i2c_write_command(i2c, addr, 0x00); // horizontal addressing
    i2c_write_command(i2c, addr, 0xB0); // page start
    i2c_write_command(i2c, addr, 0xC8); // com scan dec
    i2c_write_command(i2c, addr, 0x00);
    i2c_write_command(i2c, addr, 0x10);
    i2c_write_command(i2c, addr, 0x40);
    i2c_write_command(i2c, addr, 0x81);
    i2c_write_command(i2c, addr, 0xFF);
    i2c_write_command(i2c, addr, 0xA1);
    i2c_write_command(i2c, addr, 0xA6);
    i2c_write_command(i2c, addr, 0xA8);
    i2c_write_command(i2c, addr, height - 1);
    i2c_write_command(i2c, addr, 0xD3);
    i2c_write_command(i2c, addr, 0x00);
    i2c_write_command(i2c, addr, 0xD5);
    i2c_write_command(i2c, addr, 0xF0);
    i2c_write_command(i2c, addr, 0xD9);
    i2c_write_command(i2c, addr, 0x22);
    i2c_write_command(i2c, addr, 0xDA);
    i2c_write_command(i2c, addr, 0x12);
    i2c_write_command(i2c, addr, 0xDB);
    i2c_write_command(i2c, addr, 0x20);
    i2c_write_command(i2c, addr, 0x8D);
    i2c_write_command(i2c, addr, 0x14);
    i2c_write_command(i2c, addr, 0xAF); // display on
}

void ssd1306_clear(ssd1306_t *dev) {
    memset(dev->buffer, 0x00, sizeof(dev->buffer));
}

void ssd1306_show(ssd1306_t *dev) {
    // write buffer in pages
    for (uint8_t page = 0; page < (dev->height / 8); ++page) {
        i2c_write_command(dev->i2c, dev->address, 0xB0 + page);
        i2c_write_command(dev->i2c, dev->address, 0x00);
        i2c_write_command(dev->i2c, dev->address, 0x10);

        // prepare data buffer (control byte 0x40 + payload)
        uint8_t tx[1 + 128];
        tx[0] = 0x40;
        memcpy(&tx[1], &dev->buffer[page * dev->width], dev->width);
        i2c_write_blocking(dev->i2c, dev->address, tx, 1 + dev->width, false);
    }
}

// Very small helper to draw ASCII digits and some chars (basic implementation)
static void draw_char_simple(ssd1306_t *dev, int x, int y, char c) {
    // only support space and digits and ':' and 'C' and '%'
    const uint8_t pattern_0[] = {0x3E,0x51,0x49,0x45,0x3E};
    const uint8_t pattern_1[] = {0x00,0x42,0x7F,0x40,0x00};
    const uint8_t pattern_2[] = {0x42,0x61,0x51,0x49,0x46};
    const uint8_t pattern_3[] = {0x21,0x41,0x45,0x4B,0x31};
    const uint8_t pattern_4[] = {0x18,0x14,0x12,0x7F,0x10};
    const uint8_t pattern_5[] = {0x27,0x45,0x45,0x45,0x39};
    const uint8_t pattern_6[] = {0x3C,0x4A,0x49,0x49,0x30};
    const uint8_t pattern_7[] = {0x01,0x71,0x09,0x05,0x03};
    const uint8_t pattern_8[] = {0x36,0x49,0x49,0x49,0x36};
    const uint8_t pattern_9[] = {0x06,0x49,0x49,0x29,0x1E};
    const uint8_t pattern_colon[] = {0x00,0x36,0x36,0x00,0x00};
    const uint8_t pattern_C[] = {0x1E,0x21,0x41,0x41,0x22};
    const uint8_t pattern_pct[] = {0x62,0x64,0x08,0x13,0x23};

    const uint8_t *p = NULL;
    switch (c) {
        case '0': p = pattern_0; break;
        case '1': p = pattern_1; break;
        case '2': p = pattern_2; break;
        case '3': p = pattern_3; break;
        case '4': p = pattern_4; break;
        case '5': p = pattern_5; break;
        case '6': p = pattern_6; break;
        case '7': p = pattern_7; break;
        case '8': p = pattern_8; break;
        case '9': p = pattern_9; break;
        case ':': p = pattern_colon; break;
        case 'C': p = pattern_C; break;
        case '%': p = pattern_pct; break;
        case ' ': default: return;
    }
    // draw 5x7 into buffer (no bounds checks for brevity)
    for (int col = 0; col < 5; ++col) {
        uint8_t coldata = p[col];
        for (int row = 0; row < 7; ++row) {
            if (coldata & (1 << row)) {
                int bx = x + col;
                int by = y + row;
                if (bx < 0 || bx >= dev->width || by < 0 || by >= dev->height) continue;
                int index = (by / 8) * dev->width + bx;
                dev->buffer[index] |= (1 << (by % 8));
            }
        }
    }
}

void ssd1306_draw_string(ssd1306_t *dev, int x, int y, const char *s) {
    int cursor = x;
    for (; *s; ++s) {
        draw_char_simple(dev, cursor, y, *s);
        cursor += 6;
    }
}
