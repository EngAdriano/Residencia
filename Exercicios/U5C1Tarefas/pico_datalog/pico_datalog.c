#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/cyw43_arch.h"
#include "f_util.h"
#include "ff.h"
#include "rtc.h"
#include "aht10.h"
#include "ssd1306.h"
#include "hw_config.h"

// I2C usado: I2C0 com SDA=GPIO0, SCL=GPIO1
#define I2C_PORT0 i2c0
#define I2C_SDA0 0
#define I2C_SCL0 1
// I2C usado: I2C1 com SDA=GPIO14, SCL=GPI15
#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15

#define WIFI_SSID "ITSelf"
#define WIFI_PASSWORD "code2020"
//#define WIFI_SSID "Lu e Deza"
//#define WIFI_PASSWORD "liukin1208"

// Prototipos das funções I2C
int i2c_write(uint8_t addr, const uint8_t *data, uint16_t len);
int i2c_read(uint8_t addr, uint8_t *data, uint16_t len);
void delay_ms(uint32_t ms);

int main()
{
    stdio_init_all();
    time_init();

    // Inicializa I2C sensor
    i2c_init(I2C_PORT0, 100 * 1000); // 100 kHz
    gpio_set_function(I2C_SDA0, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL0, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA0);
    gpio_pull_up(I2C_SCL0);

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

    // Define estrutura do sensor
    AHT10_Handle aht10 = {
        .iface = {
            .i2c_write = i2c_write,
            .i2c_read = i2c_read,
            .delay_ms = delay_ms
        }
    };

    printf("Inicializando AHT10...\n");
    if (!AHT10_Init(&aht10)) {
        printf("Falha na inicialização do sensor!\n");
        ssd1306_clear();
        ssd1306_draw_string(32, 0, "Embarcatech");
        ssd1306_draw_string(23, 30, "Falha no AHT10");
        ssd1306_show();
        while (1) sleep_ms(1000);
    }

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    // Enable wifi station
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
        // Read the ip address in a human readable way
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }

    /*
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr) panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    FIL fil;
    const char* const filename = "filename.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    if (f_printf(&fil, "Hello, world!\n") < 0) {
        printf("f_printf failed\n");
    }
    fr = f_close(&fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    f_unmount(pSD->pcName);

    puts("Goodbye, world!");
    */

    while (true) {
        float temp, hum;
        if (AHT10_ReadTemperatureHumidity(&aht10, &temp, &hum)) {
            printf("Temperatura: %.2f °C | Umidade: %.2f %%\n", temp, hum);
        } else {
            printf("Falha na leitura dos dados!\n");
        }
            while(hum > 70){
                ssd1306_clear();
                ssd1306_draw_string(32, 0, "Embarcatech");
                ssd1306_draw_string(30, 10, "AHT10 Sensor");
                ssd1306_draw_string(0, 20, "Umidade");
                char hum_str[16];
                snprintf(hum_str, sizeof(hum_str), "%.2f %%", hum);
                ssd1306_draw_string(85, 20, hum_str);
                ssd1306_draw_string(22,40, "Acima de 70 %");
                ssd1306_draw_string(40,50, "ATENCAO");
                ssd1306_show();
                sleep_ms(500);
                ssd1306_draw_string(40,50, "       ");
                ssd1306_show();
                sleep_ms(500);
                AHT10_ReadTemperatureHumidity(&aht10, &temp, &hum);
            }

        while(temp < 20){
                ssd1306_clear();
                ssd1306_draw_string(32, 0, "Embarcatech");
                ssd1306_draw_string(30, 10, "AHT10 Sensor");
                ssd1306_draw_string(0, 20, "Temperatura");
                char hum_str[16];
                snprintf(hum_str, sizeof(hum_str), "%.2f C", temp);
                ssd1306_draw_string(85, 20, hum_str);
                ssd1306_draw_string(20,40, "Abaixo de 20 C");
                ssd1306_draw_string(40,50, "ATENCAO");
                ssd1306_show();
                sleep_ms(500);
                ssd1306_draw_string(40,50, "       ");
                ssd1306_show();
                sleep_ms(500);
                AHT10_ReadTemperatureHumidity(&aht10, &temp, &hum);
            }

        ssd1306_clear();
        ssd1306_draw_string(32, 0, "Embarcatech");
        ssd1306_draw_string(30, 10, "AHT10 Sensor");
        ssd1306_draw_string(0, 30, "Temperatura");
        char temp_str[16];
        snprintf(temp_str, sizeof(temp_str), "%.2f C", temp);
        ssd1306_draw_string(85, 30, temp_str);
        ssd1306_draw_string(0, 50, "Umidade");
        char hum_str[16];
        snprintf(hum_str, sizeof(hum_str), "%.2f %%", hum);
        ssd1306_draw_string(85, 50, hum_str);
        ssd1306_show();

        sleep_ms(1000);
    }
}

// Função para escrita I2C
int i2c_write(uint8_t addr, const uint8_t *data, uint16_t len) {
    int result = i2c_write_blocking(I2C_PORT0, addr, data, len, false);
    return result < 0 ? -1 : 0;
}

// Função para leitura I2C
int i2c_read(uint8_t addr, uint8_t *data, uint16_t len) {
    int result = i2c_read_blocking(I2C_PORT0, addr, data, len, false);
    return result < 0 ? -1 : 0;
}

// Função para delay
void delay_ms(uint32_t ms) {
    sleep_ms(ms);
}

