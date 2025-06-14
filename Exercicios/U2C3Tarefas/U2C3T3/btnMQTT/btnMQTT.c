#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "paho_mqtt.h"
#include "mbedtls/ssl.h"

#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

// MQTT Broker
#define MQTT_BROKER_IP "52.58.188.227"
#define MQTT_BROKER_PORT 8883
#define MQTT_CLIENT_ID "pico_w_tls"
#define MQTT_TOPIC "pico/button"
#define MQTT_USERNAME NULL
#define MQTT_PASSWORD NULL

// GPIO do botão
#define BUTTON_GPIO 14

void mqtt_message_handler(const char *topic, const char *payload) {
    printf("Recebido no tópico %s: %s\n", topic, payload);
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando MQTT TLS Pico W...\n");

    if (cyw43_arch_init()) {
        printf("Erro ao iniciar Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Conectando Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha na conexão Wi-Fi\n");
        return -1;
    }
    printf("Wi-Fi conectado!\n");

    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    // Inicia MQTT com TLS
    mqtt_client_t *client = mqtt_start_tls(
        MQTT_BROKER_IP,
        MQTT_BROKER_PORT,
        MQTT_CLIENT_ID,
        MQTT_USERNAME,
        MQTT_PASSWORD,
        mqtt_message_handler,
        "/certs/cacert.pem"
    );

    if (!client) {
        printf("Falha na conexão MQTT via TLS.\n");
        return -1;
    }

    printf("MQTT conectado com TLS!\n");

    bool last_button = false;

    while (true) {
        bool button = !gpio_get(BUTTON_GPIO);

        if (button != last_button) {
            last_button = button;

            char payload[16];
            snprintf(payload, sizeof(payload), "%s", button ? "PRESSED" : "RELEASED");

            mqtt_publish(client, MQTT_TOPIC, payload);
            printf("Publicado: %s\n", payload);
        }

        cyw43_arch_poll();
        sleep_ms(500);
    }

    cyw43_arch_deinit();
    return 0;
}
