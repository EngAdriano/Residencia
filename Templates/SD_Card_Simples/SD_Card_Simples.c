// main.c - Versão simplificada do Datalogger AHT10 -> SD card
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"

#include "aht10.h"
#include "ssd1306.h"
#include "ff.h"
#include "sd_card.h"
#include "hw_config.h"

#define I2C_PORT0 i2c0   // AHT10 (GPIO0/GPIO1)
#define I2C_SDA0 0
#define I2C_SCL0 1

#define I2C_PORT1 i2c1   // OLED SSD1306 (GPIO14/GPIO15)
#define I2C_SDA1 14
#define I2C_SCL1 15

#define LOG_FILENAME "LOG_ENV.TXT"

/* ========== Protótipos das funções auxiliares ========== */
static void delay_ms(uint32_t ms);                                      // Adapter de delay para AHT10
int i2c_write_adapter(uint8_t addr, const uint8_t *data, uint16_t len); // Adapter I2C write para AHT10
int i2c_read_adapter(uint8_t addr, uint8_t *data, uint16_t len);        // Adapter I2C read para AHT10
static bool get_timestamp_string(char *buf, size_t len);                // Timestamp helper
static void log_data(float temp, float hum);                            // Grava dados no SD (simplificado)
static void display_values(float temp, float hum, bool recording);      // Exibe valores no OLED
static void set_initial_time_if_needed(void);                           // Define hora inicial se desejar


/* ========================= main ============================ */
int main(void) {
    stdio_init_all();
    sleep_ms(100);

    // I2C sensor (I2C0)
    i2c_init(I2C_PORT0, 100 * 1000);
    gpio_set_function(I2C_SDA0, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL0, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA0);
    gpio_pull_up(I2C_SCL0);

    // I2C OLED (I2C1)
    i2c_init(I2C_PORT1, 400 * 1000);
    gpio_set_function(I2C_SDA1, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL1, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA1);
    gpio_pull_up(I2C_SCL1);

    // Opcional: definir hora inicial (descomente se quiser usar RTC)
     set_initial_time_if_needed();

    // Inicializa OLED
    ssd1306_init(I2C_PORT1);
    ssd1306_clear();
    ssd1306_draw_string(8, 14, "Inicializando...");
    ssd1306_show();
    sleep_ms(300);

    // Configura AHT10 handle (adaptadores simplificados)
    AHT10_Handle aht = {
        .iface = {
            .i2c_write = i2c_write_adapter,
            .i2c_read  = i2c_read_adapter,
            .delay_ms  = delay_ms
        }
    };

    if (!AHT10_Init(&aht)) {
        ssd1306_clear();
        ssd1306_draw_string(8, 18, "Erro AHT10");
        ssd1306_show();
        while (1) sleep_ms(500);
    }
    sleep_ms(30);

    // Se o arquivo não existir, cria cabeçalho simples
    sd_card_t *pSD = sd_get_by_num(0);
    if (pSD) {
        if (f_mount(&pSD->fatfs, pSD->pcName, 1) == FR_OK) {
            FIL f;
            if (f_open(&f, LOG_FILENAME, FA_OPEN_APPEND | FA_WRITE) == FR_OK) {
                if (f_size(&f) == 0) f_printf(&f, "DataHora; Temperatura(C); Umidade(%%)\n");
                f_close(&f);
            }
            f_unmount(pSD->pcName);
        }
    }

    ssd1306_clear();
    ssd1306_draw_string(6, 6, "Datalogger pronto");
    ssd1306_show();
    sleep_ms(600);

    // temporizadores
    const uint32_t display_interval_ms = 2000;  // 2 segundos
    const uint32_t log_interval_ms = 60 * 1000; // 1 minuto

    absolute_time_t next_display = make_timeout_time_ms(200); // primeira exibição rápida
    absolute_time_t next_log = make_timeout_time_ms(1000);    // primeiro log após 1s

    float temp = 0.0f, hum = 0.0f;

    while (1) {
        absolute_time_t now = get_absolute_time();

        if (absolute_time_diff_us(now, next_display) >= 0) {
            if (AHT10_ReadTemperatureHumidity(&aht, &temp, &hum)) {
                display_values(temp, hum, false);
            } else {
                ssd1306_clear();
                ssd1306_draw_string(6, 18, "Erro leitura");
                ssd1306_show();
            }
            next_display = delayed_by_ms(next_display, display_interval_ms);
        }

        if (absolute_time_diff_us(now, next_log) >= 0) { 
            log_data(temp, hum);
            display_values(temp, hum, true); // mostra "Gravando" por um instante
            sleep_ms(700);  // pausa para exibir status de gravação
            display_values(temp, hum, false);   // restaura display normal
            next_log = delayed_by_ms(next_log, log_interval_ms); // agenda próximo log
        }

        sleep_ms(50);
    }

    return 0;
}

/* ==================== Funções auxiliares ==================== */
/* --- Delay adapter --- */
static void delay_ms(uint32_t ms) {
     sleep_ms(ms); 
}

/* --- I2C adapter (para biblioteca AHT10) --- */
int i2c_write_adapter(uint8_t addr, const uint8_t *data, uint16_t len) {
    int r = i2c_write_blocking(I2C_PORT0, addr, data, len, false);
    return r < 0 ? -1 : 0;
}
int i2c_read_adapter(uint8_t addr, uint8_t *data, uint16_t len) {
    int r = i2c_read_blocking(I2C_PORT0, addr, data, len, false);
    return r < 0 ? -1 : 0;
}

/* --- Timestamp helper: se RTC não disponível usa ms desde boot --- */
static bool get_timestamp_string(char *buf, size_t len) {
    time_t t = time(NULL);
    if (t != (time_t)-1) {
        struct tm *tm = localtime(&t);
        if (!tm) return false;
        if (strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm) == 0) return false;
        return true;
    } else {
        uint64_t ms = to_ms_since_boot(get_absolute_time());
        snprintf(buf, len, "BOOT+%llums", (unsigned long long)ms);
        return true;
    }
}

/* --- Gravação simplificada no SD (abre/monta/grava/desmonta) --- */
static void log_data(float temp, float hum) {
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) return;

    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) return;

    FIL fil;
    fr = f_open(&fil, LOG_FILENAME, FA_OPEN_APPEND | FA_WRITE);
    if (fr != FR_OK) { f_unmount(pSD->pcName); return; }

    char ts[32];
    if (!get_timestamp_string(ts, sizeof(ts))) strncpy(ts, "UNKNOWN_TIME", sizeof(ts)-1);

    // CSV: DataHora;Temperatura;Umidade
    f_printf(&fil, "%s;%.2f;%.2f\n", ts, temp, hum);
    f_close(&fil);
    f_unmount(pSD->pcName);
}

/* --- Exibe valores no OLED (simplificado) --- */
static void display_values(float temp, float hum, bool recording) {
    char buf[32];
    ssd1306_clear();

    ssd1306_draw_string(0, 0, "Datalogger");

    snprintf(buf, sizeof(buf), "T: %.2f C", temp);
    ssd1306_draw_string(0, 20, buf);

    snprintf(buf, sizeof(buf), "H: %.2f %%", hum);
    ssd1306_draw_string(0, 34, buf);

    if (recording) ssd1306_draw_string(80, 34, "Gravando");

    // mostra hora/hora fallback (apenas HH:MM:SS quando RTC disponível)
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    if (tm) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
        ssd1306_draw_string(80, 0, buf);
    }

    ssd1306_show();
}

/* --- (Opcional) define hora inicial manualmente se desejar --- */
static void set_initial_time_if_needed(void) {
    // Comente / edite esta função se quiser que o RTC seja definido automaticamente
    datetime_t dt = {
        .year = 2025, .month = 12, .day = 10,
        .dotw = 4, .hour = 8, .min = 20, .sec = 0
    };
    rtc_init();
    rtc_set_datetime(&dt);
}