#ifndef HW_CONFIG_H
#define HW_CONFIG_H

// I2C0 - AHT10
#define I2C_PORT0 i2c0
#define I2C_SDA0 0
#define I2C_SCL0 1

// I2C1 - SSD1306
#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15

// SPI0 - SD Card (default pins confirmed)
#define SPI_PORT spi0
#define SPI_MISO 16
#define SPI_CS   17
#define SPI_SCK  18
#define SPI_MOSI 19

#endif // HW_CONFIG_H
