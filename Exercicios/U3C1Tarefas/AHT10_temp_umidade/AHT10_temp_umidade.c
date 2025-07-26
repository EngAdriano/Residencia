#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "aht10/aht10.h"

// I2C usado: I2C0 com SDA=GPIO4, SCL=GPIO5
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

// Wrapper para escrita I2C
int i2c_write_wrapper(uint8_t addr, const uint8_t *data, uint16_t len) {
    int result = i2c_write_blocking(I2C_PORT, addr, data, len, false);
    return result < 0 ? -1 : 0;
}

// Wrapper para leitura I2C
int i2c_read_wrapper(uint8_t addr, uint8_t *data, uint16_t len) {
    int result = i2c_read_blocking(I2C_PORT, addr, data, len, false);
    return result < 0 ? -1 : 0;
}

// Wrapper para delay
void delay_ms_wrapper(uint32_t ms) {
    sleep_ms(ms);
}

int main() {
    stdio_init_all();

    // Inicializa I2C
    i2c_init(I2C_PORT, 100 * 1000); // 100 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Define estrutura do sensor
    AHT10_Handle aht10 = {
        .iface = {
            .i2c_write = i2c_write_wrapper,
            .i2c_read = i2c_read_wrapper,
            .delay_ms = delay_ms_wrapper
        }
    };

    printf("Inicializando AHT10...\n");
    if (!AHT10_Init(&aht10)) {
        printf("Falha na inicialização do sensor!\n");
        while (1) sleep_ms(1000);
    }

    while (1) {
        float temp, hum;
        if (AHT10_ReadTemperatureHumidity(&aht10, &temp, &hum)) {
            printf("Temperatura: %.2f °C | Umidade: %.2f %%\n", temp, hum);
        } else {
            printf("Falha na leitura dos dados!\n");
        }
        sleep_ms(2000);
    }
}
