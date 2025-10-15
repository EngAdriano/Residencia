// main.c - Datalogger AHT10 -> SD card (log a cada 60s), OLED em tempo real
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"
#include "pico/cyw43_arch.h"

#include "aht10.h"
#include "ssd1306.h"
#include "f_util.h"
#include "ff.h"
#include "sd_card.h"      // seu helper para sd_get_by_num()
#include "hw_config.h"

// Opcional: Wi-Fi (mantive defines se quiser ativar)
#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

// I2C: AHT10 -> i2c0 (GPIO0/GPIO1), OLED -> i2c1 (GPIO14/GPIO15)
#define I2C_PORT0 i2c0
#define I2C_SDA0 0
#define I2C_SCL0 1

#define I2C_PORT1 i2c1
#define I2C_SDA1 14
#define I2C_SCL1 15

#define LOG_FILENAME "LOG_ENV.TXT"

// Protótipos
int i2c_write(uint8_t addr, const uint8_t *data, uint16_t len);
int i2c_read(uint8_t addr, uint8_t *data, uint16_t len);
void delay_ms(uint32_t ms);
void log_data(float temp, float hum);
void display_values(float temp, float hum, bool show_recording);
void display_alert(const char *msg);
bool init_wifi_and_print_ip(void);
bool get_timestamp_string(char *buf, size_t len);
void set_initial_time(void);

// -------- I2C adapter para AHT10 (usa I2C_PORT0) ----------
int i2c_write(uint8_t addr, const uint8_t *data, uint16_t len) {
    int r = i2c_write_blocking(I2C_PORT0, addr, data, len, false);
    return r < 0 ? -1 : 0;
}

int i2c_read(uint8_t addr, uint8_t *data, uint16_t len) {
    int r = i2c_read_blocking(I2C_PORT0, addr, data, len, false);
    return r < 0 ? -1 : 0;
}

void delay_ms(uint32_t ms) {
    sleep_ms(ms);
}

// -------- Timestamp helper (usa time()/localtime() com RTC) ----------
bool get_timestamp_string(char *buf, size_t len) {
    time_t t = time(NULL);
    if (t == (time_t)(-1)) {
        // fallback para ms desde boot
        uint64_t ms = to_ms_since_boot(get_absolute_time());
        snprintf(buf, len, "BOOT+%llums", (unsigned long long)ms);
        return true;
    }
    struct tm *tm = localtime(&t);
    if (!tm) return false;
    if (strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm) == 0) return false;
    return true;
}

// -------- Grava uma linha no SD ----------
void log_data(float temp, float hum) {
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("SD handle not found\n");
        return;
    }

    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }

    FIL fil;
    fr = f_open(&fil, LOG_FILENAME, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK) {
        printf("f_open(%s) error: %s (%d)\n", LOG_FILENAME, FRESULT_str(fr), fr);
        f_unmount(pSD->pcName);
        return;
    }

    char ts[32];
    if (!get_timestamp_string(ts, sizeof(ts))) {
        strncpy(ts, "UNKNOWN_TIME", sizeof(ts));
        ts[sizeof(ts)-1] = '\0';
    }

    // Formato CSV: YYYY-MM-DD HH:MM:SS;TEMP;HUM\n
    if (f_printf(&fil, "%s;%.2f;%.2f\n", ts, temp, hum) < 0) {
        printf("f_printf failed\n");
    } else {
        printf("Logged: %s;%.2f;%.2f\n", ts, temp, hum);
    }

    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    f_unmount(pSD->pcName);
}

// -------- exibe valores no OLED --------
void display_values(float temp, float hum, bool show_recording) {
    char buf[32];
    ssd1306_clear();
    ssd1306_draw_string(18, 0, "Embarcatech");
    ssd1306_draw_string(8, 10, "Datalogger AHT10");

    snprintf(buf, sizeof(buf), "T: %.2f C", temp);
    ssd1306_draw_string(0, 30, buf);

    snprintf(buf, sizeof(buf), "H: %.2f %%", hum);
    ssd1306_draw_string(0, 44, buf);

    if (show_recording) {
        ssd1306_draw_string(74, 0, "Gravando");
    }

    // exibe horário atual
    char ts[16];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    if (tm) {
        snprintf(ts, sizeof(ts), "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
        ssd1306_draw_string(80, 12, ts);
    }

    ssd1306_show();
}

// -------- alerta ----------
void display_alert(const char *msg) {
    ssd1306_clear();
    ssd1306_draw_string(16, 0, "Embarcatech");
    ssd1306_draw_string(6, 12, "ALERTA SENSOR");
    ssd1306_draw_string(0, 30, msg);
    ssd1306_show();
}

// -------- opcional: Wi-Fi init ----------
bool init_wifi_and_print_ip(void) {
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return false;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return false;
    } else {
        printf("Connected.\n");
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        return true;
    }
}

// -------- Define hora inicial manualmente ----------
void set_initial_time(void) {
    datetime_t t = {
        .year  = 2025,
        .month = 10,
        .day   = 15,
        .dotw  = 3,   // 0=domingo, 1=segunda ...
        .hour  = 14,
        .min   = 30,
        .sec   = 0
    };
    rtc_init();            // inicializa RTC
    rtc_set_datetime(&t);  // define hora inicial
}

// ========================= main ============================
int main(void) {
    stdio_init_all();
    sleep_ms(200);

    // Inicializa hora inicial
    set_initial_time();

    // I2C sensor (I2C0)
    i2c_init(I2C_PORT0, 100 * 1000); // 100 kHz
    gpio_set_function(I2C_SDA0, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL0, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA0);
    gpio_pull_up(I2C_SCL0);

    // I2C OLED (I2C1)
    i2c_init(I2C_PORT1, 400 * 1000); // 400 kHz
    gpio_set_function(I2C_SDA1, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL1, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA1);
    gpio_pull_up(I2C_SCL1);

    // Opcional: inicializa Wi-Fi (se desejar)
    init_wifi_and_print_ip();

    // Inicializa OLED
    ssd1306_init(I2C_PORT1);
    ssd1306_clear();
    ssd1306_draw_string(12, 10, "Inicializando...");
    ssd1306_show();
    sleep_ms(400);

    // Configura handle AHT10
    AHT10_Handle aht10 = {
        .iface = {
            .i2c_write = i2c_write,
            .i2c_read  = i2c_read,
            .delay_ms  = delay_ms
        }
    };

    printf("Inicializando AHT10...\n");
    if (!AHT10_Init(&aht10)) {
        printf("Falha na inicialização do AHT10\n");
        ssd1306_clear();
        ssd1306_draw_string(10, 20, "Falha AHT10");
        ssd1306_show();
        while (1) sleep_ms(1000);
    }
    sleep_ms(50); // estabiliza sensor

    // Cria cabeçalho do arquivo se necessário
    sd_card_t *pSD = sd_get_by_num(0);
    if (pSD) {
        FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
        if (fr == FR_OK) {
            FIL fil;
            fr = f_open(&fil, LOG_FILENAME, FA_OPEN_APPEND | FA_WRITE);
            if (fr == FR_OK) {
                FSIZE_t size = f_size(&fil);
                if (size == 0) {
                    f_printf(&fil, "DataHora;Temperatura(C);Umidade(%)\n");
                }
                f_close(&fil);
            }
            f_unmount(pSD->pcName);
        }
    }

    // Mostra mensagem inicial
    ssd1306_clear();
    ssd1306_draw_string(8, 10, "Datalogger pronto");
    ssd1306_draw_string(6, 30, "OLED: em tempo real");
    ssd1306_show();
    sleep_ms(800);

    // --- Agendamento ---
    const uint32_t display_interval_ms = 2000; // atualiza display a cada 2s
    const uint32_t log_interval_ms = 60 * 1000; // grava no SD a cada 60s

    absolute_time_t next_display_time = make_timeout_time_ms(500);
    absolute_time_t next_log_time = make_timeout_time_ms(1000);

    float last_temp = 0.0f;
    float last_hum  = 0.0f;

    while (true) {
        absolute_time_t now = get_absolute_time();

        // --- Atualização de display ---
        if (absolute_time_diff_us(now, next_display_time) >= 0) {
            if (AHT10_ReadTemperatureHumidity(&aht10, &last_temp, &last_hum)) {
                display_values(last_temp, last_hum, false);
            } else {
                display_alert("Erro leitura AHT10");
            }
            next_display_time = delayed_by_ms(next_display_time, display_interval_ms);
        }

        // --- Gravação no SD ---
        if (absolute_time_diff_us(now, next_log_time) >= 0) {
            log_data(last_temp, last_hum);
            display_values(last_temp, last_hum, true);
            sleep_ms(1000);
            display_values(last_temp, last_hum, false);
            next_log_time = delayed_by_ms(next_log_time, log_interval_ms);
        }

        sleep_ms(50);
    }

    return 0;
}
