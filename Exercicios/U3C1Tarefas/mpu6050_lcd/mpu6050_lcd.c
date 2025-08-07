#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "mpu6050_i2c.h"
#include <math.h>

#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15

int main()
{
    int16_t accel[3], gyro[3], temp;
    float inclination;

    // Inicializa a biblioteca padrão do Pico
    stdio_init_all();

    // Inicializa I2C OLED
    i2c_init(I2C_PORT1, 400000);
    gpio_set_function(I2C_SDA1, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL1, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA1);
    gpio_pull_up(I2C_SCL1);

    // Inicializa o display OLED
    ssd1306_init(I2C_PORT1);
    ssd1306_clear();
    ssd1306_draw_string(32, 0, "Embarcatech");
    ssd1306_draw_string(20, 10, "Inicializando...");
    ssd1306_show();
    sleep_ms(1000);

    // Inicializa I2C MPU6050
    mpu6050_setup_i2c();
    mpu6050_reset();
    mpu6050_set_accel_range(0); // Set to ±2g

    while (true) {
        // Lê os dados do MPU6050
        mpu6050_read_raw(accel, gyro, &temp);
        // Converte os dados para g e graus/s
        float accel_g[3];
        float gyro_dps[3];
        accel_g[0] = accel[0] / ACCEL_SENS_2G;
        accel_g[1] = accel[1] / ACCEL_SENS_2G;
        accel_g[2] = accel[2] / ACCEL_SENS_2G;
        gyro_dps[0] = gyro[0] / 131.0f; // 131.0f is the sensitivity for ±250 degrees/s
        gyro_dps[1] = gyro[1] / 131.0f;
        gyro_dps[2] = gyro[2] / 131.0f;

        inclination = atan2(gyro_dps[0], sqrt(gyro_dps[1] * gyro_dps[1] + gyro_dps[2] * gyro_dps[2])) * (180.0 / M_PI);

        // Exibe os dados no OLED
        ssd1306_clear();    
        ssd1306_draw_string(0, 0, "Acelerometro (g):");
        //ssd1306_draw_string(0, 10, "X:"); 
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%.2f", accel_g[0]);
        ssd1306_draw_string(0, 10, buffer);
        //ssd1306_draw_string(0, 20, "Y:");
        snprintf(buffer, sizeof(buffer), "%.2f", accel_g[1]);
        ssd1306_draw_string(45, 10, buffer);
        //ssd1306_draw_string(0, 30, "Z:");
        snprintf(buffer, sizeof(buffer), "%.2f", accel_g[2]);
        ssd1306_draw_string(90, 10, buffer);
        ssd1306_draw_string(0, 25, "Giroscopio (dps):");
        //ssd1306_draw_string(0, 50, "X:");
        snprintf(buffer, sizeof(buffer), "%.2f", gyro_dps[0]);
        ssd1306_draw_string(0, 35, buffer);
        //ssd1306_draw_string(0, 60, "Y:");
        snprintf(buffer, sizeof(buffer), "%.2f", gyro_dps[1]);
        ssd1306_draw_string(45, 35, buffer);
        //ssd1306_draw_string(0, 70, "Z:");
        snprintf(buffer, sizeof(buffer), "%.2f", gyro_dps[2]);
        ssd1306_draw_string(90, 35, buffer);
        //ssd1306_draw_string(0, 50, "Temp:");
        //snprintf(buffer, sizeof(buffer), "%.2f", (temp / 340.0f) + 36.53f);
        //ssd1306_draw_string(35, 50, buffer);
        snprintf(buffer, sizeof(buffer), "%.2f", inclination);
        ssd1306_draw_string(0, 50, "Inclinacao:");
        ssd1306_draw_string(80, 50, buffer);
        ssd1306_show();
        // Aguarda um pouco antes de ler novamente
        sleep_ms(100);
    }
}
