#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lib/vlx53l0x/VL53L0X.h"

// Conexões do sensor
#define I2C_ID        i2c0
#define I2C_SCL_PIN   0
#define I2C_SDA_PIN   1

#define BAUD_RATE     400000   // 400kHz

int main()
{
    stdio_init_all();

    // Inicia a interface I2C
  uint baud = i2c_init (I2C_ID, BAUD_RATE);
  printf ("I2C @ %u Hz\n", baud);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SCL_PIN);
  gpio_pull_up(I2C_SDA_PIN);

  VL53L0X sensor;
  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();

    while (true) {
        
        printf("millimeters: %d\n", sensor.readRangeContinuousMillimeters());
        sleep_ms(1000);
    }
}
