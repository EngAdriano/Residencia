/* -------------------------------------------------------------------------------------------------------------------------------------
/ Projeto: bh1750_servo
/ Descrição: Este projeto utiliza um sensor de luminosidade BH1750 para controlar a posição de um servo contínuo simulado, 
/ exibindo informações em um display OLED SSD1306. O servo é calibrado automaticamente no boot se um botão for pressionado, 
/ e o tempo de rotação é salvo na memória flash para uso futuro. A luminosidade ambiente determina a posição do servo: menos 
/ de 100 lux move o servo para 0 graus, entre 100 e 200 lux para 90 graus, e acima de 200 lux para 180 graus. Os valores dos 
/ ângulos são aproximados pois o servo não é um padrão de posição.
/ Bibliotecas: pico-sdk, extras (servo_sim, flash_storage, ssd1306, bh1750).
/ Autor: José Adriano
/ Data de Criação: 06/09/2025
/----------------------------------------------------------------------------------------------------------------------------------------
*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "servo_sim.h"
#include "flash_storage.h"
#include "ssd1306.h"
#include "bh1750.h"

// ==== Pinos ====
#define SERVO_PIN   2       // GPIO do servo contínuo (simulado)    
#define BTN_CALIB   5       // Botão de calibração (ativo em nível baixo)

// I2C BH1750 (i2c0)
#define I2C_PORT_SENSOR i2c0        
#define SDA_SENSOR 0
#define SCL_SENSOR 1

// I2C SSD1306 (i2c1)   
#define I2C_PORT_OLED i2c1
#define SDA_OLED 14
#define SCL_OLED 15

int main() {
    stdio_init_all();

    // Botão de calibração (ativo em nível baixo)
    gpio_init(BTN_CALIB);
    gpio_set_dir(BTN_CALIB, GPIO_IN);
    gpio_pull_up(BTN_CALIB);

    // I2C BH1750
    i2c_init(I2C_PORT_SENSOR, 100 * 1000);
    gpio_set_function(SDA_SENSOR, GPIO_FUNC_I2C);
    gpio_set_function(SCL_SENSOR, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_SENSOR);
    gpio_pull_up(SCL_SENSOR);
    bh1750_init(I2C_PORT_SENSOR);
    sleep_ms(200);

    // I2C SSD1306
    i2c_init(I2C_PORT_OLED, 400000);
    gpio_set_function(SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_OLED);
    gpio_pull_up(SCL_OLED);
    ssd1306_init(I2C_PORT_OLED);

    // Tela inicial
    ssd1306_clear();
    ssd1306_draw_string(18, 0, "Embarcatech Servo");
    ssd1306_draw_string(8, 12, "Inicializando...");
    ssd1306_show();

    // Servo: carrega calibração da flash (ou usa 1000ms padrão)
    uint32_t rotation_time_ms = 1000;
    bool have_calib = flash_storage_read(&rotation_time_ms);

    servo_sim_t servo;
    servo_sim_init(&servo, SERVO_PIN, (float)rotation_time_ms);

    // Se botão pressionado no boot → calibrar e salvar
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

    while (true) {
        // Lê luminosidade
        float lux = bh1750_read_lux(I2C_PORT_SENSOR);

        // Mapeia para 0/90/180 graus
        float angle = (lux < 100) ? 0.0f : (lux < 200) ? 90.0f : 180.0f;

        // Move (simulado)
        servo_sim_set_angle(&servo, angle);

        // Display
        ssd1306_clear();
        ssd1306_draw_string(18, 0, "Embarcatech Servo");
        char line1[24], line2[24], line3[24];
        snprintf(line1, sizeof(line1), "Lux: %.1f", lux);
        snprintf(line2, sizeof(line2), "Alvo: %.0f deg", angle);
        snprintf(line3, sizeof(line3), "t180: %ums", (uint32_t)(180.0f / servo.deg_per_ms));
        ssd1306_draw_string(6, 20, line1);
        ssd1306_draw_string(6, 36, line2);
        ssd1306_draw_string(6, 52, line3);
        ssd1306_show();

        sleep_ms(1200);
    }
}
