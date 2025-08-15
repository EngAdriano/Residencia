#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "bh1750.h"
#include "ssd1306.h"
#include "servo_sg90.h"

#define I2C_PORT i2c0
#define SDA_PIN 0
#define SCL_PIN 1

#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15


int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 100 * 1000);  // 100 kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Inicializa I2C OLED
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

    sleep_ms(100); // Aguarda estabilização

    bh1750_init(I2C_PORT);

    // Inicializa o servo na GPIO 2 com pulsos de 500 a 2500 microsegundos
    servo_t servo;
    servo_init(&servo, 2, 500, 2500);

    while (true) {
        float lux = bh1750_read_lux(I2C_PORT);
        printf("Luminosidade: %.2f lux\n", lux);

        ssd1306_clear();
        ssd1306_draw_string(32, 0, "Embarcatech");
        ssd1306_draw_string(20, 10, "Sensor - BH1750");
        ssd1306_draw_string(30, 30, "Luminosidade");
        char temp_str[16];
        snprintf(temp_str, sizeof(temp_str), "%.2f Lux", lux);
        ssd1306_draw_string(35, 50, temp_str);
        ssd1306_show();
        sleep_ms(1000);

        // Teste do servo alterar para executar o movimento do exercício
        for (int ang = 0; ang <= 180; ang += 10) {
            servo_set_angle(&servo, ang);
            sleep_ms(300);
        }
        for (int ang = 180; ang >= 0; ang -= 10) {
            servo_set_angle(&servo, ang);
            sleep_ms(300);
        }
    }
}


