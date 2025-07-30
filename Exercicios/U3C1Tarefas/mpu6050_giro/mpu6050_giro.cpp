#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "MPU6050.h"
#include "ssd1306.h"

#define I2C_PORT i2c0
#define SDA_PIN 0
#define SCL_PIN 1

#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15

int main()
{
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
  // Inicia o sensor
  MPU6050 sensor (I2C_PORT);
  sensor.begin();

  // Mostra o ID do sensor
  printf ("ID do Sensor: %02X\n", sensor.getId());

  // Laço Principal
  MPU6050::VECT_3D data;
  while(1) {
    sleep_ms(2000);
    sensor.getAccel(&data);
    printf ("Aceleracao X:%.1f Y:%.1f Z:%.1f\n", data.x, data.y, data.z);
    sensor.getGyro(&data);
    printf ("Giroscopio X:%.1f Y:%.1f Z:%.1f\n", data.x, data.y, data.z);
    printf ("Temperatura: %.1f\n", sensor.getTemp());
    printf("\n");
  }
}




