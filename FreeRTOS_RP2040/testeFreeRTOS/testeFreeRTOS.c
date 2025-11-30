#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

// ----------------------------------------------------------
//  PROTÓTIPOS
// ----------------------------------------------------------
void task_hello(void *pvParameters);
void task_wifi_reconnect(void *pvParameters);
void wifi_initial_connect(void);

// ----------------------------------------------------------
//  MAIN
// ----------------------------------------------------------
int main()
{
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar o CYW43\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    // ------------------------------------------------------
    //  Conecta antes de iniciar o FreeRTOS
    // ------------------------------------------------------
    wifi_initial_connect();

    // ------------------------------------------------------
    //  Cria as tasks após WiFi já estar OK
    // ------------------------------------------------------
    xTaskCreate(task_hello, "HelloTask", 2048, NULL, 1, NULL);
    xTaskCreate(task_wifi_reconnect, "WiFiReconnect", 4096, NULL, 2, NULL);

    // Inicia o scheduler
    vTaskStartScheduler();

    while (1);
    return 0;
}

// ----------------------------------------------------------
//  FUNÇÃO DE CONEXÃO INICIAL (BLOQUEANTE)
// ----------------------------------------------------------
void wifi_initial_connect(void)
{
    const char *ssid = "Lu e Deza";
    const char *pass = "liukin1208";

    while (1)
    {
        printf("Tentando conectar ao WiFi...\n");

        cyw43_arch_lwip_begin();
        int result = cyw43_arch_wifi_connect_timeout_ms(
            ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 20000
        );
        cyw43_arch_lwip_end();

        if (result == 0)
        {
            uint8_t *ip = (uint8_t*)&cyw43_state.netif[0].ip_addr.addr;
            printf("WiFi conectado! IP: %d.%d.%d.%d\n",
                   ip[0], ip[1], ip[2], ip[3]);
            break;
        }

        printf("Falha! Tentando novamente em 3s...\n");
        sleep_ms(3000);
    }
}


// ----------------------------------------------------------
//  TASK: RECONEXÃO AUTOMÁTICA
// ----------------------------------------------------------
void task_wifi_reconnect(void *pvParameters)
{
    const char *ssid = "Lu e Deza";
    const char *pass = "liukin1208";

    while (1)
    {
        // Se perdeu o IP → reconectar
        if (!cyw43_state.netif[0].ip_addr.addr)
        {
            printf("WiFi caiu! Tentando reconectar...\n");

            cyw43_arch_lwip_begin();
            int result = cyw43_arch_wifi_connect_timeout_ms(
                ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 20000
            );
            cyw43_arch_lwip_end();

            if (result == 0)
            {
                uint8_t *ip = (uint8_t*)&cyw43_state.netif[0].ip_addr.addr;
                printf("Reconectado! IP: %d.%d.%d.%d\n",
                      ip[0], ip[1], ip[2], ip[3]);
            }
            else
            {
                printf("Falha na reconexão. Tentando em 5s...\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // verifica a cada 5 segundos
    }
}

// ----------------------------------------------------------
//  TASK: HELLO WORLD PERIÓDICO
// ----------------------------------------------------------
void task_hello(void *pvParameters)
{
    while (1)
    {
        printf("Hello World via FreeRTOS!\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
