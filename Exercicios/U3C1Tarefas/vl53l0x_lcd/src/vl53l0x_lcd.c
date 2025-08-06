#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "vl53l0x.h"
#include "ssd1306.h"

#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15

int main()
{
    stdio_init_all();
    
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

    config_i2c();       // Configuração de I2C
    sleep_ms(2000);

    if (!vl53l0x_init(I2C_PORT)) {
        //printf("Falha ao inicializar o VL53L0X.\n");
        ssd1306_clear();
        ssd1306_draw_string(32, 0, "Embarcatech");
        ssd1306_draw_string(20, 10, "Sensor - VL53L0X");
        ssd1306_draw_string(0, 30, "Erro de inicializacao");
        ssd1306_draw_string(5, 40, "Verifique conexoes");
        ssd1306_show();
        while (true) {
            sleep_ms(1000); // Loop infinito em caso de erro
        }
    }

    while (true) {
        int distancia = vl53l0x_read_distance_mm(I2C_PORT);

        ssd1306_clear();
        ssd1306_draw_string(32, 0, "Embarcatech");
        ssd1306_draw_string(20, 10, "Sensor - VL53L0X");
        

        
        if (distancia < 0) {
            printf("Erro na leitura da distância.\n");
        } else {
            //printf("Distância: %d mm (%.2f m)\n", distancia, distancia / 1000.0f);
            ssd1306_draw_string(5, 30, "Distancia em mm e m");
            char temp_str[16];
            snprintf(temp_str, sizeof(temp_str), "%d mm (%.2f m)\n", distancia, distancia / 1000.0f);
            ssd1306_draw_string(20, 50, temp_str);
        }
        ssd1306_show();
        sleep_ms(1000);
    }
}
