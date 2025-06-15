#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

// 🛜 Wi-Fi credentials
#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

// 🔗 MQTT Broker (usando hostname)
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_BROKER_PORT 1883  // ⚠️ Nome alterado para evitar conflito

// 🚦 GPIO do botão
#define BUTTON_GPIO 5

// Variáveis globais
static mqtt_client_t *mqtt_client;
static bool button_last_state = false;
static ip_addr_t broker_ip;
static bool mqtt_connected = false;

// 🟢 Callback de conexão MQTT
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Conectado ao broker MQTT!\n");
        mqtt_connected = true;
    } else {
        printf("Falha na conexão MQTT. Código: %d\n", status);
        mqtt_connected = false;
    }
}

// 🚀 Publica o estado do botão
void publish_button_state(bool pressed) {
    if (!mqtt_connected) return;

    const char *topic = "pico/button";
    const char *message = pressed ? "ON" : "OFF";

    err_t err = mqtt_publish(mqtt_client, topic, message, strlen(message), 0, 0, NULL, NULL);
    if (err == ERR_OK) {
        printf("Publicado: %s\n", message);
    } else {
        printf("Erro ao publicar: %d\n", err);
    }
}

// 🔍 Callback de DNS (nome ajustado para não conflitar)
void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr != NULL) {
        broker_ip = *ipaddr;
        printf("DNS resolvido: %s -> %s\n", name, ipaddr_ntoa(ipaddr));

        // Informações da conexão MQTT
        struct mqtt_connect_client_info_t ci = {
            .client_id = "pico-client",
            .keep_alive = 60,
            .client_user = NULL,
            .client_pass = NULL,
            .will_topic = NULL,
            .will_msg = NULL,
            .will_qos = 0,
            .will_retain = 0
        };

        // 🔗 Conectar ao broker MQTT
        mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);
    } else {
        printf("Falha ao resolver DNS para %s\n", name);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando...\n");

    // 🛜 Inicializar Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro na inicialização do Wi-Fi\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro na conexão Wi-Fi\n");
        return -1;
    } else {
        printf("Wi-Fi conectado com sucesso!\n");
    }

    // 🚦 Configurar GPIO do botão
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_down(BUTTON_GPIO);

    // 🔧 Criar cliente MQTT
    mqtt_client = mqtt_client_new();

    // 🔍 Resolver DNS e conectar
    err_t err = dns_gethostbyname(MQTT_BROKER, &broker_ip, dns_found_cb, NULL);
    if (err == ERR_OK) {
        // DNS já estava no cache
        dns_found_cb(MQTT_BROKER, &broker_ip, NULL);
    } else if (err == ERR_INPROGRESS) {
        printf("Resolvendo DNS...\n");
    } else {
        printf("Erro ao iniciar resolução DNS: %d\n", err);
        return -1;
    }

    // 🔁 Loop principal
    while (true) {
        bool button_state = gpio_get(BUTTON_GPIO);

        if (button_state != button_last_state) {
            publish_button_state(button_state);
            button_last_state = button_state;
        }

        sleep_ms(500);
    }

    // 🚪 Encerrar (não chega aqui normalmente)
    cyw43_arch_deinit();
    return 0;
}
