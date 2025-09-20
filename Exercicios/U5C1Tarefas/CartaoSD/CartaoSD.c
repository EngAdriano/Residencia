#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "sd_card.h"

// Definições SPI
// São utilizados SPI 0, e alocamos para os seguintes pinos GPIO
// Os pinos podem ser alterados, consulte a tabela de seleção de função GPIO no datasheet para obter informações sobre atribuições de GPIO
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// Definições I2C
// Este exemplo utilizará I2C0 em GPIO8 (SDA) e GPIO9 (SCL) operando a 400KHz.
// Os pinos podem ser alterados, consulte a tabela de seleção de função GPIO no datasheet para obter informações sobre atribuições de GPIO
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9



int main()
{
    stdio_init_all();

    // Inicialização SPI. Este exemplo utilizará SPI a 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Se o Chip select é ativo-baixo, então o inicializaremos em um estado alto
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // Para mais exemplos de uso do SPI, consulte https://github.com/raspberrypi/pico-examples/tree/master/spi

    // Inicialização I2C. Utilizando a 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Para mais exemplos de uso do I2C, consulte https://github.com/raspberrypi/pico-examples/tree/master/i2c

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
