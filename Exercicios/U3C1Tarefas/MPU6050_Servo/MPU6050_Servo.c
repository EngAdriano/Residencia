#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "servo_sim.h"
#include "flash_storage.h"
#include "ssd1306.h"
#include "mpu6050_i2c.h"

// ==== Pinos ====
#define SERVO_PIN   2       // GPIO do servo contínuo (simulado)
#define BTN_CALIB   5       // Botão de calibração (ativo em nível baixo)

// I2C OLED (i2c1)
#define I2C_PORT_OLED i2c1
#define SDA_OLED 14
#define SCL_OLED 15

int main() {
    stdio_init_all();

    // === Botão de calibração ===
    gpio_init(BTN_CALIB);
    gpio_set_dir(BTN_CALIB, GPIO_IN);
    gpio_pull_up(BTN_CALIB);

    // === Inicializa MPU6050 (I2C0) ===
    mpu6050_setup_i2c();
    mpu6050_reset();
    sleep_ms(200);

    if (!mpu6050_test()) {
        printf("MPU6050 nao encontrado!\n");
        while (1) {
            sleep_ms(1000);
        }
    }

    // === Inicializa OLED (I2C1) ===
    i2c_init(I2C_PORT_OLED, 400000);
    gpio_set_function(SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_OLED);
    gpio_pull_up(SCL_OLED);
    ssd1306_init(I2C_PORT_OLED);

    // Tela inicial
    ssd1306_clear();
    ssd1306_draw_string(20, 0, "Servo MPU6050");
    ssd1306_draw_string(8, 12, "Inicializando...");
    ssd1306_show();
    sleep_ms(1000);

    // === Servo: carrega calibração ===
    uint32_t rotation_time_ms = 1000;
    bool have_calib = flash_storage_read(&rotation_time_ms);

    servo_sim_t servo;
    servo_sim_init(&servo, SERVO_PIN, (float)rotation_time_ms);

    if (!gpio_get(BTN_CALIB)) {
        ssd1306_clear();
        ssd1306_draw_string(20, 24, "Calibrando...");
        ssd1306_show();

        servo_sim_calibrate(&servo);
        rotation_time_ms = (uint32_t)(180.0f / servo.deg_per_ms);
        flash_storage_write(rotation_time_ms);

        ssd1306_clear();
        ssd1306_draw_string(8, 24, "Calibracao salva!");
        ssd1306_show();
        sleep_ms(1200);
    } else if (have_calib) {
        char msg[24];
        snprintf(msg, sizeof(msg), "Calib: %ums", rotation_time_ms);
        ssd1306_clear();
        ssd1306_draw_string(10, 24, msg);
        ssd1306_show();
        sleep_ms(800);
    }

    float current_angle = 90.0f;  // posição inicial

    while (true) {
        int16_t accel[3], gyro[3], temp_raw;
        mpu6050_read_raw(accel, gyro, &temp_raw); // ainda lemos temp_raw mas ignoramos

        // Escolha de ângulo: simples → aceleração no eixo X
        float ax = accel[0] / ACCEL_SENS_2G; // normalizado em "g"
        float target_angle = (ax < -0.5f) ? 0.0f : (ax < 0.5f ? 90.0f : 180.0f);

        // Movimento suave → incrementa gradualmente
        if (current_angle < target_angle) current_angle += 2.0f;
        else if (current_angle > target_angle) current_angle -= 2.0f;

        // Serial debug
        printf("AX=%.2fg AY=%.2fg AZ=%.2fg | GX=%d GY=%d GZ=%d | Alvo=%.0f deg | Atual=%.0f deg\n",
               accel[0]/ACCEL_SENS_2G, accel[1]/ACCEL_SENS_2G, accel[2]/ACCEL_SENS_2G,
               gyro[0], gyro[1], gyro[2], target_angle, current_angle);

        // Servo simulado
        servo_sim_set_angle(&servo, current_angle);

        // Display
        ssd1306_clear();
        ssd1306_draw_string(20, 0, "Servo MPU6050");

        char line1[24], line2[24];
        snprintf(line1, sizeof(line1), "AX: %.2fg", accel[0]/ACCEL_SENS_2G);
        snprintf(line2, sizeof(line2), "Ang: %.0f/%0.f", current_angle, target_angle);

        ssd1306_draw_string(6, 20, line1);
        ssd1306_draw_string(6, 36, line2);
        ssd1306_show();

        sleep_ms(80); // controle da velocidade do movimento
    }
}
