/* ----------------------------------------------------------------------------------------------------------------
/ Projeto: Botão + Controle de LED via MQTT
/ Descrição: Publica o estado do botão (GPIO5) e recebe comandos para ligar/desligar um LED (GPIO12) via MQTT.
/ Autor: José Adriano
/ Data: 22/06/2025
/ ----------------------------------------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

// =================== Configurações ======================
#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_BROKER_PORT 1883

#define MQTT_TOPIC_PUBLISH "pico/button"
#define MQTT_TOPIC_SUBSCRIBE "pico/led"

#define BUTTON_GPIO 5
#define LED_GPIO 12

// =================== Variáveis Globais ==================
static mqtt_client_t *mqtt_client;
static ip_addr_t broker_ip;
static bool mqtt_connected = false;
static bool button_last_state = false;

// =================== Protótipos ========================
static void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void publish_button_state(bool pressed);
void dns_check_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg);

// =================== Função Principal ===================
int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("\n=== Iniciando MQTT Button + LED ===\n");

    // Inicializa Wi-Fi
    if (cyw43_arch_init()) {
        printf("[Wi-Fi] Erro na inicialização\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    printf("[Wi-Fi] Conectando...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("[Wi-Fi] Falha na conexão Wi-Fi\n");
        return -1;
    } else {
        printf("[Wi-Fi] Conectado com sucesso!\n");
    }

    // Configura GPIO do botão
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    // Configura GPIO do LED
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
    gpio_put(LED_GPIO, 0); // Desliga inicialmente

    // Inicializa cliente MQTT
    mqtt_client = mqtt_client_new();

    // Resolve DNS do broker
    err_t err = dns_gethostbyname(MQTT_BROKER, &broker_ip, dns_check_callback, NULL);
    if (err == ERR_OK) {
        dns_check_callback(MQTT_BROKER, &broker_ip, NULL);
    } else if (err == ERR_INPROGRESS) {
        printf("[DNS] Resolvendo...\n");
    } else {
        printf("[DNS] Erro DNS: %d\n", err);
        return -1;
    }

    // Loop principal
    while (true) {
        cyw43_arch_poll();

        // Lê o estado do botão
        bool button_state = !gpio_get(BUTTON_GPIO);

        if (button_state != button_last_state) {
            printf("[BOTÃO] Estado mudou para: %s\n", button_state ? "ON" : "OFF");
            publish_button_state(button_state);
            button_last_state = button_state;
        }

        sleep_ms(200);
    }

    cyw43_arch_deinit();
    return 0;
}

// =================== MQTT - Callback de Conexão ===================
static void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] Conectado!\n");
        mqtt_connected = true;

        // Configura callbacks de recebimento
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

        // Faz inscrição no tópico de controle do LED
        err_t err = mqtt_subscribe(client, MQTT_TOPIC_SUBSCRIBE, 0, NULL, NULL);
        if (err == ERR_OK) {
            printf("[MQTT] Inscrito no tópico: %s\n", MQTT_TOPIC_SUBSCRIBE);
        } else {
            printf("[MQTT] Erro na inscrição: %d\n", err);
        }

    } else {
        printf("[MQTT] Falha na conexão. Código: %d\n", status);
        mqtt_connected = false;
    }
}

// =================== MQTT - Callback de Publicação ===================
void publish_button_state(bool pressed) {
    if (!mqtt_connected) {
        printf("[MQTT] Não conectado, não publicando\n");
        return;
    }

    const char *message = pressed ? "ON" : "OFF";

    printf("[MQTT] Publicando: %s -> %s\n", MQTT_TOPIC_PUBLISH, message);

    err_t err = mqtt_publish(mqtt_client, MQTT_TOPIC_PUBLISH, message, strlen(message), 0, 0, NULL, NULL);
    if (err == ERR_OK) {
        printf("[MQTT] Publicação enviada\n");
    } else {
        printf("[MQTT] Erro ao publicar: %d\n", err);
    }
}

// =================== MQTT - Callback de Mensagens Recebidas ===================
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("[MQTT] Mensagem recebida no tópico: %s (tamanho: %lu)\n", topic, (unsigned long)tot_len);
}

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char msg[len + 1];
    memcpy(msg, data, len);
    msg[len] = '\0';

    printf("[MQTT] Dados recebidos: %s\n", msg);

    if (strcmp(msg, "ON") == 0) {
        gpio_put(LED_GPIO, 1);
        printf("[LED] Ligado\n");
    } else if (strcmp(msg, "OFF") == 0) {
        gpio_put(LED_GPIO, 0);
        printf("[LED] Desligado\n");
    } else {
        printf("[LED] Comando desconhecido: %s\n", msg);
    }
}

// =================== DNS Callback ===================
void dns_check_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr != NULL) {
        broker_ip = *ipaddr;
        printf("[DNS] Resolvido: %s -> %s\n", name, ipaddr_ntoa(ipaddr));

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

        printf("[MQTT] Conectando...\n");
        mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_callback, NULL, &ci);
    } else {
        printf("[DNS] Falha na resolução DNS para %s\n", name);
    }
}
