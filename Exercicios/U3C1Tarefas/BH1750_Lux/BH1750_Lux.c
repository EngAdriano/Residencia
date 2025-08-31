#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bh1750.h"
#include "ssd1306.h"
#include "servo_sg90.h"

#define ANGLE_ALERT_THRESHOLD 60.0f // Ângulo limite para alerta
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180

// ==== Configurações I2C ====
// BH1750 em i2c0
#define I2C_PORT_SENSOR i2c0
#define SDA_SENSOR 0
#define SCL_SENSOR 1

// SSD1306 em i2c1
#define I2C_PORT_OLED i2c1
#define SDA_OLED 14
#define SCL_OLED 15

// ==== Configuração Servo ====
#define SERVO_PIN 2

int main() {
    stdio_init_all();

    // ---- Inicializa BH1750 ----
    i2c_init(I2C_PORT_SENSOR, 100 * 1000);
    gpio_set_function(SDA_SENSOR, GPIO_FUNC_I2C);
    gpio_set_function(SCL_SENSOR, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_SENSOR);
    gpio_pull_up(SCL_SENSOR);

    bh1750_init(I2C_PORT_SENSOR);
    sleep_ms(200); // tempo para estabilizar

    // ---- Inicializa SSD1306 ----
    i2c_init(I2C_PORT_OLED, 400000);
    gpio_set_function(SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_OLED);
    gpio_pull_up(SCL_OLED);

    ssd1306_init(I2C_PORT_OLED);
    ssd1306_clear();
    ssd1306_draw_string(32, 0, "Embarcatech");
    ssd1306_draw_string(20, 10, "Inicializando...");
    ssd1306_show();

    // ---- Inicializa Servo NG90 ----
    servo_t servo;
    servo_init(&servo, SERVO_PIN, 500, 2500); // pulsos de 500–2500 µs

    sleep_ms(500);

    while (true) {

        
        // ---- Leitura do sensor ----
        float lux = bh1750_read_lux(I2C_PORT_SENSOR);
        printf("Luminosidade: %.2f lux\n", lux);

        // ---- Calcular ângulo proporcional ----
        // lux = 0   → 0°
        // lux = 500 → 90°
        // lux >=1000 → 180°
        float angle;
        if (lux <= 0) {
            angle = 0;
        } else if (lux >= 1000) {
            angle = 180;
        } else {
            angle = (lux / 1000.0f) * 180.0f;
        }

        // ---- Atualizar servo ----
        servo_set_angle(&servo, angle);

        // ---- Atualizar display ----
        ssd1306_clear();
        ssd1306_draw_string(32, 0, "Embarcatech");
        ssd1306_draw_string(20, 12, "Sensor - BH1750");
        ssd1306_draw_string(30, 28, "Luminosidade");

        char lux_str[20];
        snprintf(lux_str, sizeof(lux_str), "%.1f Lux", lux);
        ssd1306_draw_string(25, 45, lux_str);

        char ang_str[20];
        snprintf(ang_str, sizeof(ang_str), "Servo: %.0f°", angle);
        ssd1306_draw_string(25, 56, ang_str);

        ssd1306_show();

        sleep_ms(1000);
    }

}
