// Integra AHT10 + SSD1306 usando FreeRTOS no Raspberry Pi Pico W
// Baseado nos arquivos do usuário (ssd1306.*, aht10.* e font6x8.h).

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "ssd1306.h"
#include "aht10.h"

// ======================
// Definições de hardware
// ======================
#define BUTTON_PIN 5
#define LED_PIN    11

// I2C para AHT10 -> i2c0 (GPIO 0 = SDA, GPIO1 = SCL)
#define I2C_PORT0     i2c0
#define I2C_SDA0      0
#define I2C_SCL0      1

// I2C para SSD1306 -> i2c1 (GPIO14 = SDA, GPIO15 = SCL)
#define I2C_PORT1     i2c1
#define I2C_SDA1      14
#define I2C_SCL1      15

// ======================
// Tipos / Queues
// ======================
typedef struct {
    float temperature;
    float humidity;
} AHTReading_t;

static QueueHandle_t queueButton;
static QueueHandle_t queueAHT;

// ======================
// AHT10 handle global
// ======================
static AHT10_Handle ahtDev;

// ======================
// Wrappers I2C / delay para AHT10
// Esses callbacks satisfazem AHT10_Interface em aht10.h
// ======================
static int aht_i2c_write(uint8_t addr, const uint8_t *data, uint16_t len) {
    // i2c_write_blocking returns number of bytes written
    int wrote = i2c_write_blocking(I2C_PORT0, addr, data, len, false);
    return (wrote == (int)len) ? 0 : -1; // 0 = sucesso (conforme aht10.c)
}

static int aht_i2c_read(uint8_t addr, uint8_t *data, uint16_t len) {
    int read = i2c_read_blocking(I2C_PORT0, addr, data, len, false);
    return (read == (int)len) ? 0 : -1;
}

static void aht_delay_ms(uint32_t ms) {
    // usa SDK do Pico (bloqueante). Pode ser trocado por vTaskDelay se quiser não bloqueante,
    // mas a interface do driver pede uma função de delay simples.
    sleep_ms(ms);
}

// ======================
// Prototypes das tasks
// ======================
void task_button(void *pvParameters);
void task_led(void *pvParameters);
void task_aht10(void *pvParameters);
void task_oled(void *pvParameters);

// ======================
// main
// ======================
int main(void)
{
    stdio_init_all();

    // Inicializa I2C AHT10 (i2c0)
    i2c_init(I2C_PORT0, 100 * 1000); // 100 kHz
    gpio_set_function(I2C_SDA0, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL0, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA0);
    gpio_pull_up(I2C_SCL0);

    // Inicializa I2C OLED (i2c1)
    i2c_init(I2C_PORT1, 400 * 1000); // 400 kHz
    gpio_set_function(I2C_SDA1, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL1, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA1);
    gpio_pull_up(I2C_SCL1);

    // Inicializa display (usa ssd1306.c)
    ssd1306_init(I2C_PORT1);
    ssd1306_clear();
    ssd1306_draw_string(10, 0, "Inicializando...");
    ssd1306_show();

    // Se desejar usar Wi-Fi (Pico W) inicialize cyw43 aqui como no seu exemplo original
    if (cyw43_arch_init()) {
        printf("Falha na inicialização do Wi-Fi\n");
        // não retorna; pode prosseguir sem Wi-Fi
    } else {
        cyw43_arch_enable_sta_mode();
    }

    // Cria queues
    queueButton = xQueueCreate(5, sizeof(uint8_t));
    queueAHT    = xQueueCreate(3, sizeof(AHTReading_t));

    if (queueButton == NULL || queueAHT == NULL) {
        printf("Erro ao criar queues\n");
        while (1) tight_loop_contents();
    }

    // Configura handle do AHT10 com callbacks
    ahtDev.iface.i2c_write = aht_i2c_write;
    ahtDev.iface.i2c_read  = aht_i2c_read;
    ahtDev.iface.delay_ms  = aht_delay_ms;
    ahtDev.initialized = false;

    // Tenta inicializar sensor AHT10 (pode falhar se fiação errada)
    bool ok = AHT10_Init(&ahtDev);
    if (!ok) {
        printf("AHT10 não inicializado\n");
        ssd1306_clear();
        ssd1306_draw_string(6, 0, "AHT10 erro");
        ssd1306_draw_string(6, 10, "Verifique I2C");
        ssd1306_show();
    } else {
        printf("AHT10 init ok\n");
    }

    // Cria tasks FreeRTOS
    xTaskCreate(task_button, "Button", 1024, NULL, 3, NULL);
    xTaskCreate(task_led,    "LED",    1024, NULL, 3, NULL);
    xTaskCreate(task_aht10,  "AHT10",  2048, NULL, 2, NULL);
    xTaskCreate(task_oled,   "OLED",   2048, NULL, 1, NULL);

    // Inicia scheduler
    vTaskStartScheduler();

    // Nunca chega aqui
    while (1) tight_loop_contents();
    return 0;
}

// ======================
// task_button: envia evento quando botão é pressionado
// ======================
void task_button(void *pvParameters)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    uint8_t last = 1;
    uint8_t msg = 1;

    while (1) {
        uint8_t state = gpio_get(BUTTON_PIN);
        if (state == 0 && last == 1) {
            xQueueSend(queueButton, &msg, 0);
            vTaskDelay(pdMS_TO_TICKS(200)); // debounce simples
        }
        last = state;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ======================
// task_led: alterna LED ao receber evento do botão
// ======================
void task_led(void *pvParameters)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint8_t rx;
    uint8_t ledState = 0;

    while (1) {
        if (xQueueReceive(queueButton, &rx, portMAX_DELAY)) {
            ledState = !ledState;
            gpio_put(LED_PIN, ledState);
            printf("LED %s\n", ledState ? "ON" : "OFF");
        }
    }
}

// ======================
// task_aht10: lê sensor e envia leituras para queueAHT
// ======================
void task_aht10(void *pvParameters)
{
    AHTReading_t reading;         // variável para enviar pela queue
    for (;;) {
        // Lê só se inicializado
        float temp = 0.0f, hum = 0.0f;
        bool ok = AHT10_ReadTemperatureHumidity(&ahtDev, &temp, &hum);
        if (!ok) {
            // se falhar, marca com NaN (ou mantenha último valor)
            reading.temperature = -999.0f;
            reading.humidity = -999.0f;
            printf("AHT10 leitura falhou\n");
        } else {
            reading.temperature = temp;
            reading.humidity = hum;
            printf("AHT10: T=%.2f C  H=%.2f %%\n", temp, hum);
        }

        // envia leitura (não bloqueante)
        xQueueOverwrite(queueAHT, &reading); // sempre mantém último valor
        vTaskDelay(pdMS_TO_TICKS(2000)); // leitura a cada 2s
    }
}

// ======================
// task_oled: recebe leitura e atualiza display
// ======================
void task_oled(void *pvParameters)
{
    AHTReading_t rcv;               // variável para receber da queue
    char line1[24], line2[24];      // linhas formatadas para display

    // Mensagem inicial
    ssd1306_clear();
    ssd1306_draw_string(10, 0, "AHT10 + OLED");
    ssd1306_draw_string(6, 10, "Iniciando...");
    ssd1306_show();
    vTaskDelay(pdMS_TO_TICKS(1000));

    for (;;) {
        // espera por nova leit. ou timeout para atualizar a cada 2s
        if (xQueueReceive(queueAHT, &rcv, pdMS_TO_TICKS(2500))) {
            ssd1306_clear();
            if (rcv.temperature == -999.0f) {
                ssd1306_draw_string(6, 0, "Leitura AHT10");
                ssd1306_draw_string(6, 10, "Falha!");
            } else {
                // Formata as linhas
                snprintf(line1, sizeof(line1), "T: %6.2f C", rcv.temperature);
                snprintf(line2, sizeof(line2), "H: %6.2f %%", rcv.humidity);

                ssd1306_draw_string(0, 0, "AHT10 Leitura:");
                ssd1306_draw_string(0, 12, line1);
                ssd1306_draw_string(0, 24, line2);
            }
            ssd1306_show();
        } else {
            // timeout sem nova mensagem: opcionalmente refresca tela ou ignora
        }
    }
}
