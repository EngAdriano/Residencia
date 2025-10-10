#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "pico/cyw43_arch.h"
#include "hw_config.h"
#include "ssd1306.h"


#define WIFI_SSID "ITSelf"
#define WIFI_PASSWORD "code2020"

int main()
{
    stdio_init_all();
    time_init();
    i2c_init(I2C_PORT0, 100 * 1000);
    gpio_set_function(I2C_SDA0, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL0, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA0);
    gpio_pull_up(I2C_SCL0);

    i2c_init(I2C_PORT1, 400000);
    gpio_set_function(I2C_SDA1, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL1, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA1);
    gpio_pull_up(I2C_SCL1);

    ssd1306_init(I2C_PORT1);
    ssd1306_clear();
    ssd1306_draw_string(32, 0, "Embarcatech");
    ssd1306_draw_string(20, 10, "Inicializando...");
    ssd1306_show();

    while (true) {
        
    }
}
