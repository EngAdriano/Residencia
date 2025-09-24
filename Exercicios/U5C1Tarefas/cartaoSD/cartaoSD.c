#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "lib/FatFs_SPI/sd_driver/sd_card.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19



int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

int main() {
    stdio_init_all();
    sd_card_t sd;

    if (!sd_init(&sd)) {
        printf("Falha ao montar o SD!\n");
        return 1;
    }

    // Escrever
    if (sd_open_file(&sd, "teste.txt", FA_WRITE | FA_CREATE_ALWAYS)) {
        sd_write_file(&sd, "Hello, SD Card!\n");
        sd_close_file(&sd);
        printf("Arquivo escrito!\n");
    }

    // Ler
    char buffer[128];
    if (sd_open_file(&sd, "teste.txt", FA_READ)) {
        if (sd_read_file(&sd, buffer, sizeof(buffer))) {
            printf("Conteúdo lido: %s\n", buffer);
        }
        sd_close_file(&sd);
    }

    sd_deinit(&sd);
    return 0;
}

