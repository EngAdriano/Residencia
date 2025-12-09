#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Bibliotecas enviadas pelo usuário
#include "aht10.h"      
#include "ssd1306.h"    

// =======================================================
// CONFIGURAÇÃO I2C
// =======================================================
#define I2C_PORT_AHT  i2c0
#define SDA_AHT       0
#define SCL_AHT       1

#define I2C_PORT_OLED i2c1
#define SDA_OLED      14
#define SCL_OLED      15

// =======================================================
// QUEUE DE COMUNICAÇÃO ENTRE AS TASKS
// =======================================================
typedef struct {
    float temperature;
    float humidity;
} SensorData_t;

QueueHandle_t xSensorQueue;

// =======================================================
// WRAPPERS PARA O DRIVER DO AHT10
// =======================================================
int i2c_write_wrapper(uint8_t addr, const uint8_t *data, uint16_t len)
{
    int r = i2c_write_blocking(I2C_PORT_AHT, addr, data, len, false);
    return (r < 0) ? -1 : 0;
}

int i2c_read_wrapper(uint8_t addr, uint8_t *data, uint16_t len)
{
    int r = i2c_read_blocking(I2C_PORT_AHT, addr, data, len, false);
    return (r < 0) ? -1 : 0;
}

void delay_ms_wrapper(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// =======================================================
// TASK — SENSOR AHT10
// =======================================================
void TaskSensor(void *pv)
{
    AHT10_Handle aht10 = {
        .iface = {
            .i2c_write = i2c_write_wrapper,
            .i2c_read  = i2c_read_wrapper,
            .delay_ms  = delay_ms_wrapper
        }
    };

    printf("Inicializando AHT10...\n");

    if (!AHT10_Init(&aht10)) {
        printf("FALHA ao inicializar AHT10!\n");
        while (1) vTaskDelay(1000);
    }

    printf("AHT10 pronto!\n");

    while (1)
    {
        float t, h;

        if (AHT10_ReadTemperatureHumidity(&aht10, &t, &h))
        {
            SensorData_t data = {
                .temperature = t,
                .humidity = h
            };

            // Mantém sempre o último valor (não bloqueia)
            xQueueOverwrite(xSensorQueue, &data);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));  // Leitura a cada 1s
    }
}

// =======================================================
// TASK — DISPLAY OLED
// =======================================================
void TaskDisplay(void *pv)
{
    SensorData_t data;
    char temp_str[16];
    char hum_str[16];

    while (1)
    {
        // Lê SEM CONSUMIR a fila
        if (xQueuePeek(xSensorQueue, &data, portMAX_DELAY))
        {
            float t = data.temperature;
            float h = data.humidity;

            ssd1306_clear();
            ssd1306_draw_string(32, 0, "Embarcatech");
            ssd1306_draw_string(30, 10, "AHT10 Sensor");

            // ALERTA 1 — umidade alta
            if (h > 70.0)
            {
                snprintf(hum_str, sizeof(hum_str), "%.2f %%", h);
                ssd1306_draw_string(0, 20, "Umidade:");
                ssd1306_draw_string(85, 20, hum_str);
                ssd1306_draw_string(22, 40, "Acima de 70%");
                ssd1306_draw_string(40, 50, "ATENCAO");
                ssd1306_show();
                vTaskDelay(pdMS_TO_TICKS(300));
                continue;
            }

            // ALERTA 2 — temperatura baixa
            if (t < 20.0)
            {
                snprintf(temp_str, sizeof(temp_str), "%.2f C", t);
                ssd1306_draw_string(0, 20, "Temp:");
                ssd1306_draw_string(85, 20, temp_str);
                ssd1306_draw_string(20, 40, "Abaixo de 20C");
                ssd1306_draw_string(40, 50, "ATENCAO");
                ssd1306_show();
                vTaskDelay(pdMS_TO_TICKS(300));
                continue;
            }

            // MODO NORMAL
            snprintf(temp_str, sizeof(temp_str), "%.2f C", t);
            snprintf(hum_str, sizeof(hum_str), "%.2f %%", h);

            ssd1306_draw_string(0, 30, "Temperatura");
            ssd1306_draw_string(85, 30, temp_str);

            ssd1306_draw_string(0, 50, "Umidade");
            ssd1306_draw_string(85, 50, hum_str);

            ssd1306_show();
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// =======================================================
// TASK — SERIAL (UART DEBUG)
// =======================================================
void TaskSerial(void *pv)
{
    SensorData_t data;

    while (1)
    {
        if (xQueuePeek(xSensorQueue, &data, portMAX_DELAY))
        {
            printf("Temperatura: %.2f C | Umidade: %.2f %%\n",
                   data.temperature,
                   data.humidity);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// =======================================================
// MAIN — CONFIGURA SISTEMA E INICIA FREERTOS
// =======================================================
int main()
{
    stdio_init_all();

    // -------------------
    // I2C DO SENSOR
    // -------------------
    i2c_init(I2C_PORT_AHT, 100000);
    gpio_set_function(SDA_AHT, GPIO_FUNC_I2C);
    gpio_set_function(SCL_AHT, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_AHT);
    gpio_pull_up(SCL_AHT);

    // -------------------
    // I2C DO OLED
    // -------------------
    i2c_init(I2C_PORT_OLED, 400000);
    gpio_set_function(SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(SCL_OLED, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_OLED);
    gpio_pull_up(SCL_OLED);

    // Inicializa OLED
    ssd1306_init(I2C_PORT_OLED);

    // -------------------
    // CRIA A QUEUE
    // -------------------
    xSensorQueue = xQueueCreate(1, sizeof(SensorData_t));
    xQueueReset(xSensorQueue);

    // -------------------
    // CRIA AS TASKS
    // -------------------
    xTaskCreate(TaskSensor,  "Sensor",  1024, NULL, 3, NULL);
    xTaskCreate(TaskDisplay, "Display", 1024, NULL, 1, NULL);
    xTaskCreate(TaskSerial,  "Serial",  1024, NULL, 1, NULL);

    // -------------------
    // INICIA O FREERTOS
    // -------------------
    vTaskStartScheduler();

    while (true) {}
}
