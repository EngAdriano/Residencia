#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

// Configurações do Wi-Fi
#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

// IP do servidor UDP
#define SERVER_IP "192.168.1.28"
#define SERVER_PORT 34567

// Pinos do joystick
#define ADC_PIN_X 26 // GP26 -> ADC0
#define ADC_PIN_Y 27 // GP27 -> ADC1

#define LED_PIN 12 // LED integrado no Raspberry Pi Pico

// Prototipos de funções
const char* direcao_geografica(uint16_t x, uint16_t y);
void send_udp(struct udp_pcb *pcb, const char *msg);

// Função principal
int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando...\n");
    // Configurar LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1); // Ligar LED

    // Inicializar ADC
    adc_init();
    adc_gpio_init(ADC_PIN_X);
    adc_gpio_init(ADC_PIN_Y);

    // Inicializar Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha na conexão Wi-Fi\n");
        return -1;
    }

    printf("Wi-Fi conectado!\n");

    // Inicializar UDP
    struct udp_pcb *pcb = udp_new();
    if (!pcb) {
        printf("Erro ao criar PCB UDP\n");
        return -1;
    }

    while (true) {
        // Ler eixo X
        adc_select_input(0);
        uint16_t x = adc_read();

        // Ler eixo Y
        adc_select_input(1);
        uint16_t y = adc_read();

        // Obter direção
        const char *direcao = direcao_geografica(x, y);

        // Criar mensagem
        char msg[100];
        snprintf(msg, sizeof(msg), "X:%d,Y:%d,Direcao:%s", x, y, direcao);
        printf("Enviando: %s\n", msg);

        // Enviar via UDP
        send_udp(pcb, msg);

        sleep_ms(500);

    }

    cyw43_arch_deinit();
    return 0;
}

// Função para obter direção na rosa dos ventos
const char* direcao_geografica(uint16_t x, uint16_t y) {
    const uint16_t centro = 2048;
    const uint16_t zona_central = 500;

    int deltaX = x - centro;
    int deltaY = y - centro;

    if (abs(deltaX) < zona_central && abs(deltaY) < zona_central) {
        return "Centro";
    }

    if (deltaY > zona_central) {
        if (deltaX > zona_central) return "Nordeste";
        else if (deltaX < -zona_central) return "Noroeste";
        else return "Norte";
    } else if (deltaY < -zona_central) {
        if (deltaX > zona_central) return "Sudeste";
        else if (deltaX < -zona_central) return "Sudoeste";
        else return "Sul";
    } else {
        if (deltaX > zona_central) return "Leste";
        else if (deltaX < -zona_central) return "Oeste";
    }

    return "Centro";
}

// Função para enviar dados via UDP
void send_udp(struct udp_pcb *pcb, const char *msg) {
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_RAM);
    if (!p) return;
    memcpy(p->payload, msg, strlen(msg));

    ip_addr_t dest_ip;
    ipaddr_aton(SERVER_IP, &dest_ip);

    udp_sendto(pcb, p, &dest_ip, SERVER_PORT);
    pbuf_free(p);
}

