#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bh1750.h"
#include "ssd1306.h"
#include "servo_velocity.h"

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

    servo_velocity_t servo;
    servo_init(&servo, 2, 0.02f);  // GPIO2, ganho Kp=0.02

    uint32_t last_sensor_time = 0;
    uint32_t last_display_time = 0;

    while (true) {
        // Atualiza servo a cada ~20ms
        servo_update(&servo);
        sleep_ms(20);

        uint32_t now = to_ms_since_boot(get_absolute_time());

        // ---- Leitura do sensor (a cada 1s) ----
        if (now - last_sensor_time >= 1000) {
            last_sensor_time = now;
            float lux = bh1750_read_lux(I2C_PORT_SENSOR);
            printf("Luminosidade: %.2f lux\n", lux);

            // ---- Calcular ângulo proporcional ----
            float angle;
            if (lux <= 0) {
                angle = 0;
            } else if (lux >= 100) {
                angle = 180;
            } else {
                angle = (lux / 1000.0f) * 180.0f;
            }

            servo_set_target_angle(&servo, angle);

            // Atualiza OLED junto com leitura
            ssd1306_clear();
            ssd1306_draw_string(32, 0, "Embarcatech");
            ssd1306_draw_string(20, 12, "Sensor - BH1750");
            ssd1306_draw_string(30, 28, "Luminosidade");

            char lux_str[20];
            snprintf(lux_str, sizeof(lux_str), "%.1f Lux", lux);
            ssd1306_draw_string(25, 45, lux_str);

            char ang_str[20];
            snprintf(ang_str, sizeof(ang_str), "Servo: %.0f", angle);
            ssd1306_draw_string(25, 56, ang_str);

            ssd1306_show();
        }
    }
}