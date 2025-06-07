#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/tcp.h"

// Wi-Fi
#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

// Webhook.site - Servidor em nuvem para receber dados
#define WEBHOOK_HOST "webhook.site"
#define WEBHOOK_PATH "/32a77e20-204e-46f1-b195-b5632fbd9e4c"
#define WEBHOOK_PORT 80

// Hardware
#define BUTTON_PIN 5
#define ADC_TEMP 4

float ler_temperatura() {
    adc_select_input(ADC_TEMP);
    uint16_t raw = adc_read();
    float volts = raw * 3.3f / (1 << 12);
    return 27.0f - (volts - 0.706f) / 0.001721f;
}

const char* ler_botao() {
    return gpio_get(BUTTON_PIN) ? "LIBERADO" : "PRESSIONADO";
}

void enviar_http_post(const char* payload) {
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (!pcb) {
        printf("Erro ao criar TCP PCB\n");
        return;
    }

    ip_addr_t ip_dest;
    err_t dns_err = dns_gethostbyname(WEBHOOK_HOST, &ip_dest, NULL, NULL);
    if (dns_err != ERR_OK) {
        printf("Erro DNS (%d)\n", dns_err);
        tcp_abort(pcb);
        return;
    }

    err_t err = tcp_connect(pcb, &ip_dest, WEBHOOK_PORT, NULL);
    if (err != ERR_OK) {
        printf("Erro ao conectar TCP (%d)\n", err);
        tcp_abort(pcb);
        return;
    }

    sleep_ms(500); // espera estabilizar

    char requisicao[512];
    snprintf(requisicao, sizeof(requisicao),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        WEBHOOK_PATH, WEBHOOK_HOST, strlen(payload), payload
    );

    err = tcp_write(pcb, requisicao, strlen(requisicao), TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        tcp_output(pcb);
        printf("📤 Enviado: %s\n", payload);
    } else {
        printf("Erro ao escrever TCP: %d\n", err);
    }

    sleep_ms(500);
    tcp_close(pcb);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando...\n");

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    adc_init();
    adc_set_temp_sensor_enabled(true);

    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, WIFI_PASSWORD,
            CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("❌ Falha ao conectar ao Wi-Fi\n");
        return -1;
    }

    printf("✅ Wi-Fi conectado!\n");

    while (true) {
        float temp = ler_temperatura();
        const char* botao = ler_botao();

        char payload[128];
        snprintf(payload, sizeof(payload),
                 "Temperatura: %.2f C, Botao: %s", temp, botao);

        enviar_http_post(payload);
        sleep_ms(5000); // aguarda 5s
    }

    cyw43_arch_deinit();
    return 0;
}
