#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "lwip/apps/sntp.h"

#include "MQTTClient.h"
#include "Network.h"
#include "NetworkTLS.h"

#define WIFI_SSID "Lu e Deza"
#define WIFI_PASS "liukin1208"

#define MQTT_BROKER "52.58.188.227"
#define MQTT_PORT 8883
#define MQTT_TOPIC "pico/button"
#define MQTT_CLIENT_ID "PicoW_Button"

#define BUTTON_GPIO 5
#define MQTT_BUF_SIZE 100

void sync_time() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    printf("Sincronizando horário...\n");
    absolute_time_t timeout = make_timeout_time_ms(10000);
    while (!time_reached(timeout)) {
        sleep_ms(500);
        if (sntp_get_current_timestamp() > 100000) {
            printf("Horário sincronizado.\n");
            return;
        }
    }
    printf("Falha na sincronização de horário.\n");
}

bool conectar_wifi() {
    if (cyw43_arch_init()) {
        printf("Erro na inicialização do Wi-Fi.\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha na conexão Wi-Fi.\n");
        return false;
    }

    const ip4_addr_t *ip = &cyw43_state.netif[0].ip_addr;
    printf("Conectado! IP: %s\n", ip4addr_ntoa(ip));
    return true;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando MQTT Button...\n");

    if (!conectar_wifi()) {
        return -1;
    }

    sync_time();

    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    Network network;
    MQTTClient client;
    unsigned char sendbuf[MQTT_BUF_SIZE], readbuf[MQTT_BUF_SIZE];

    NetworkInit(&network);
    printf("Conectando ao broker MQTT...\n");
    if (NetworkConnectTLS(&network, MQTT_BROKER, MQTT_PORT,
                           "/certs/AmazonRootCA1.pem", NULL, NULL)) {
        printf("Erro na conexão TLS MQTT.\n");
        return -1;
    }
    printf("Conectado ao broker.\n");

    MQTTClientInit(&client, &network, 5000, sendbuf, MQTT_BUF_SIZE, readbuf, MQTT_BUF_SIZE);

    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = MQTT_CLIENT_ID;

    if (MQTTConnect(&client, &connectData) != 0) {
        printf("Erro na conexão MQTT.\n");
        return -1;
    }
    printf("Conectado no MQTT.\n");

    int ultimo_estado = 1;

    while (true) {
        int estado = gpio_get(BUTTON_GPIO);

        if (estado != ultimo_estado) {
            ultimo_estado = estado;

            char payload[50];
            snprintf(payload, sizeof(payload), "{\"botao\": \"%s\"}", estado == 0 ? "pressionado" : "solto");

            MQTTMessage message;
            message.qos = QOS0;
            message.retained = 0;
            message.payload = payload;
            message.payloadlen = strlen(payload);

            if (MQTTPublish(&client, MQTT_TOPIC, &message) == 0) {
                printf("Publicado: %s\n", payload);
            } else {
                printf("Erro ao publicar MQTT.\n");
            }
        }

        MQTTYield(&client, 100);
        sleep_ms(200);
    }

    MQTTDisconnect(&client);
    NetworkDisconnect(&network);
    cyw43_arch_deinit();
    return 0;
}
