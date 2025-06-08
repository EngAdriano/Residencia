#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

#define ADC_PIN_X 26
#define ADC_PIN_Y 27
#define LED_PIN 12

#define THINGSPEAK_HOST "api.thingspeak.com"
#define THINGSPEAK_PORT 80
#define API_KEY "D18MS6D04L89UNKS"

typedef struct {
    char requisicao[512];
} http_get_context_t;

err_t tcp_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        printf("Conexão encerrada.\n");
        tcp_close(tpcb);
        free(arg);
        return ERR_OK;
    }

    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    pbuf_copy_partial(p, buffer, p->tot_len, 0);
    printf("Resposta do servidor:\n%s\n", buffer);

    pbuf_free(p);
    tcp_close(tpcb);
    free(arg);
    return ERR_OK;
}

err_t tcp_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Erro ao conectar: %d\n", err);
        tcp_abort(tpcb);
        free(arg);
        return err;
    }

    http_get_context_t *ctx = (http_get_context_t *)arg;
    err_t wr_err = tcp_write(tpcb, ctx->requisicao, strlen(ctx->requisicao), TCP_WRITE_FLAG_COPY);
    if (wr_err != ERR_OK) {
        printf("Erro ao escrever TCP: %d\n", wr_err);
        tcp_abort(tpcb);
        free(ctx);
        return wr_err;
    }

    tcp_output(tpcb);
    tcp_recv(tpcb, tcp_recv_cb);
    return ERR_OK;
}

void enviar_para_thingspeak(uint16_t x, uint16_t y) {
    ip_addr_t dest_ip;
    http_get_context_t *ctx = malloc(sizeof(http_get_context_t));
    if (!ctx) return;

    snprintf(ctx->requisicao, sizeof(ctx->requisicao),
        "GET /update?api_key=%s&field1=%d&field2=%d HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        API_KEY, x, y, THINGSPEAK_HOST);

    err_t dns_err = dns_gethostbyname(THINGSPEAK_HOST, &dest_ip, NULL, NULL);
    if (dns_err == ERR_OK) {
        struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
        if (!pcb) {
            free(ctx);
            return;
        }
        tcp_arg(pcb, ctx);
        tcp_connect(pcb, &dest_ip, THINGSPEAK_PORT, tcp_connected_cb);
    } else {
        printf("Erro de DNS: %d\n", dns_err);
        free(ctx);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando...\n");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    adc_init();
    adc_gpio_init(ADC_PIN_X);
    adc_gpio_init(ADC_PIN_Y);

    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return -1;
    }

    printf("Wi-Fi conectado!\n");

    while (true) {
        adc_select_input(0);
        uint16_t x = adc_read();

        adc_select_input(1);
        uint16_t y = adc_read();

        printf("Enviando para ThingSpeak: X=%d, Y=%d\n", x, y);
        enviar_para_thingspeak(x, y);

        sleep_ms(1000);
    }

    cyw43_arch_deinit();
    return 0;
}