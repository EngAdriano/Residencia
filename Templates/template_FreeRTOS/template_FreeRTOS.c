/****************************************************
 *            PICO W + FreeRTOS + SSD1306
 *      SOMENTE TASKS E MAIN NESTE ARQUIVO
 *   (SSD1306 continua em arquivos separados)
 ****************************************************/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Biblioteca SSD1306 externa
#include "ssd1306.h"

// I2C configuration (usando sua escolha)
#define I2C_PORT   i2c1
#define SDA_PIN    14
#define SCL_PIN    15

/****************************************************
 *                PROTÓTIPOS DAS TASKS
 ****************************************************/
void wifi_task(void *pv);
void display_task(void *pv);
void heartbeat_task(void *pv);

/****************************************************
 *                VARIÁVEIS GLOBAIS
 ****************************************************/
SemaphoreHandle_t xDisplaySem;   // sinaliza quando atualizar display
SemaphoreHandle_t xStatusMutex;  // protege as mensagens

char wifi_status_msg[64];
char wifi_ip_msg[32];

/****************************************************
 *                PROTÓTIPOS DE FUNÇÕES
 ****************************************************/
void wifi_task(void *pv);
void display_task(void *pv);
void heartbeat_task(void *pv);

/****************************************************
 *                     MAIN
 ****************************************************/
int main() 
{
    stdio_init_all();

    // Configura I2C1
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Cria semáforos
    xDisplaySem = xSemaphoreCreateBinary();
    xStatusMutex = xSemaphoreCreateMutex();

    // Inicializa display (biblioteca externa)
    ssd1306_init(I2C_PORT);

    // Mensagens iniciais
    strcpy(wifi_status_msg, "Iniciando...");
    strcpy(wifi_ip_msg, "----");

    xSemaphoreGive(xDisplaySem);

    // Cria tasks
    xTaskCreate(wifi_task, "wifi", 4096, NULL, 2, NULL);
    xTaskCreate(display_task, "display", 3072, NULL, 1, NULL);
    xTaskCreate(heartbeat_task, "beat", 1024, NULL, 1, NULL);

    // Start FreeRTOS
    vTaskStartScheduler();

    while (true) tight_loop_contents();
}


/****************************************************
 *                WIFI TASK
 ****************************************************/
#define WIFI_SSID      "Lu e Deza"
#define WIFI_PASSWORD  "liukin1208"

void wifi_task(void *pv) 
{
    if (cyw43_arch_init()) {
        strcpy(wifi_status_msg, "ERRO WiFi INIT");
        xSemaphoreGive(xDisplaySem);
        vTaskDelete(NULL);
    }

    cyw43_arch_enable_sta_mode();

    for (;;) 
    {
        xSemaphoreTake(xStatusMutex, portMAX_DELAY);
        strcpy(wifi_status_msg, "Conectando...");
        strcpy(wifi_ip_msg, "----");
        xSemaphoreGive(xStatusMutex);
        xSemaphoreGive(xDisplaySem);

        int r = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID,
            WIFI_PASSWORD,
            CYW43_AUTH_WPA2_AES_PSK,
            15000);

        if (r == 0) {
            // Obtem IP
            uint8_t *ip = (uint8_t*)&cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;

            xSemaphoreTake(xStatusMutex, portMAX_DELAY);
            strcpy(wifi_status_msg, "Conectado");
            snprintf(wifi_ip_msg, sizeof(wifi_ip_msg),
                     "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            xSemaphoreGive(xStatusMutex);

            xSemaphoreGive(xDisplaySem);

            vTaskDelay(pdMS_TO_TICKS(6000));
        }
        else {
            xSemaphoreTake(xStatusMutex, portMAX_DELAY);
            strcpy(wifi_status_msg, "Falha WiFi");
            strcpy(wifi_ip_msg, "----");
            xSemaphoreGive(xStatusMutex);

            xSemaphoreGive(xDisplaySem);

            vTaskDelay(pdMS_TO_TICKS(4000));
        }
    }
}

/****************************************************
 *                DISPLAY TASK
 ****************************************************/
void display_task(void *pv) 
{
    for (;;) 
    {
        xSemaphoreTake(xDisplaySem, portMAX_DELAY);

        char st[64];
        char ip[32];

        xSemaphoreTake(xStatusMutex, portMAX_DELAY);
        strcpy(st, wifi_status_msg);
        strcpy(ip, wifi_ip_msg);
        xSemaphoreGive(xStatusMutex);

        ssd1306_clear();
        ssd1306_draw_string(20, 0, "Pico W FreeRTOS");
        ssd1306_draw_string(0, 15, "Status: ");
        ssd1306_draw_string(50, 15, st);
        ssd1306_draw_string(0, 35, "IP: ");
        ssd1306_draw_string(20, 35, ip);
        ssd1306_show();
    }
}

/****************************************************
 *                HEARTBEAT TASK
 ****************************************************/
void heartbeat_task(void *pv)
{
    for (;;) 
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(800));
    }
}
