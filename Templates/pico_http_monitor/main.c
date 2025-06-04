
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/netdb.h"

#define WIFI_SSID "SEU_WIFI"
#define WIFI_PASSWORD "SUA_SENHA"

#define SERVER_IP "192.168.1.100"
#define SERVER_PORT 5000

#define BOTAO_PIN 15
#define SENSOR_ADC_PIN 26

void send_http_post(bool botao_status, float sensor_valor) {
    int sock;
    struct sockaddr_in server_addr;
    char send_buffer[512];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Erro ao criar socket\n");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = ipaddr_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        printf("Erro na conexão com servidor\n");
        close(sock);
        return;
    }

    snprintf(send_buffer, sizeof(send_buffer),
        "POST /endereco_api HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "{\"botao\":%s,\"sensor\":%.2f}\r\n",
        SERVER_IP,
        29 + (botao_status ? 4 : 5) + 6,
        botao_status ? "true" : "false",
        sensor_valor
    );

    write(sock, send_buffer, strlen(send_buffer));

    char recv_buf[256];
    int len;
    do {
        len = read(sock, recv_buf, sizeof(recv_buf) - 1);
        if (len > 0) {
            recv_buf[len] = 0;
            printf("%s", recv_buf);
        }
    } while (len > 0);

    close(sock);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando...\n");

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL)) {
        printf("Erro ao iniciar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro ao conectar Wi-Fi\n");
        return -1;
    } else {
        printf("Conectado ao Wi-Fi!\n");
    }

    gpio_init(BOTAO_PIN);
    gpio_set_dir(BOTAO_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_PIN);

    adc_init();
    adc_gpio_init(SENSOR_ADC_PIN);
    adc_select_input(0);

    while (true) {
        bool botao_pressionado = !gpio_get(BOTAO_PIN);
        uint16_t adc_raw = adc_read();
        float adc_volts = adc_raw * 3.3f / 4095.0f;

        printf("Botao: %d, Sensor: %.2f V\n", botao_pressionado, adc_volts);

        send_http_post(botao_pressionado, adc_volts);

        sleep_ms(1000);
    }

    cyw43_arch_deinit();
    return 0;
}
