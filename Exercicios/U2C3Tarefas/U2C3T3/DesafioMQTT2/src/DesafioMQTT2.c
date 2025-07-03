#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

#include "ssd1306.h"

// ======================= Configurações =========================
#define WIFI_SSID "Lu e Deza"
#define WIFI_PASSWORD "liukin1208"

#define MQTT_BROKER "broker.emqx.io"       // Broker da MQTTX
#define MQTT_PORT_BROKER 1883              // Porta sem TLS

#define MQTT_TOPIC_PUB "embarca/status"
#define MQTT_TOPIC_SUB "embarca/comando"

#define BUTTON_GPIO 5
#define LED_GPIO 12

#define I2C_SDA 14
#define I2C_SCL 15

// ======================= Variáveis Globais =====================
static mqtt_client_t *mqtt_client;
static ip_addr_t broker_ip;
static bool mqtt_connected = false;
static bool button_last_state = false;

// ======================= Protótipos ============================
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void publish_button_state(bool pressed);
void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg);

// ======================= Função Principal ======================
int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("\n=== Iniciando ===\n");

    // Inicializa Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro na inicialização Wi-Fi\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    printf("[Wi-Fi] Conectando...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("[Wi-Fi] Falha na conexão Wi-Fi\n");
        return -1;
    }
    printf("[Wi-Fi] Conectado!\n");

    // Inicializa I2C e OLED
    i2c_init(i2c1, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(i2c1);
    ssd1306_clear();
    ssd1306_draw_string(0, 0, "Wi-Fi OK");
    ssd1306_draw_string(0, 10, "Conectando MQTT");
    ssd1306_show();

    // Configura Botão
    gpio_init(BUTTON_GPIO);
    gpio_set_dir(BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO);

    // Configura LED
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    // Inicializa MQTT
    mqtt_client = mqtt_client_new();
    err_t err = dns_gethostbyname(MQTT_BROKER, &broker_ip, dns_found_cb, NULL);
    if (err == ERR_OK) {
        dns_found_cb(MQTT_BROKER, &broker_ip, NULL);
    } else if (err == ERR_INPROGRESS) {
        printf("[DNS] Resolvendo...\n");
    } else {
        printf("[DNS] Erro DNS: %d\n", err);
        return -1;
    }

    // Loop Principal
    while (true) {
        cyw43_arch_poll();

        bool button_state = !gpio_get(BUTTON_GPIO);
        if (button_state != button_last_state) {
            printf("[BOTAO] %s\n", button_state ? "ON" : "OFF");
            publish_button_state(button_state);

            ssd1306_clear();
            ssd1306_draw_string(0, 0, "Botao:");
            ssd1306_draw_string(50, 0, button_state ? "ON" : "OFF");
            ssd1306_draw_string(0, 10, "LED:");
            ssd1306_draw_string(50, 10, gpio_get(LED_GPIO) ? "ON" : "OFF");
            ssd1306_show();

            button_last_state = button_state;
        }

        sleep_ms(200);
    }

    cyw43_arch_deinit();
    return 0;
}

// ================== Callbacks e Publicação =====================
void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr != NULL) {
        broker_ip = *ipaddr;
        printf("[DNS] %s -> %s\n", name, ipaddr_ntoa(ipaddr));

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
        mqtt_client_connect(mqtt_client, &broker_ip, MQTT_PORT_BROKER, mqtt_connection_cb, NULL, &ci);
    } else {
        printf("[DNS] Falha na resolução\n");
    }
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] Conectado!\n");
        mqtt_connected = true;

        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

        err_t err = mqtt_subscribe(client, MQTT_TOPIC_SUB, 0, NULL, NULL);
        if (err != ERR_OK) {
            printf("[MQTT] Erro subscribe: %d\n", err);
        } else {
            printf("[MQTT] Inscrito no topico %s\n", MQTT_TOPIC_SUB);
        }

        ssd1306_clear();
        ssd1306_draw_string(0, 0, "MQTT Conectado");
        ssd1306_show();
    } else {
        printf("[MQTT] Falha conexão: %d\n", status);
        mqtt_connected = false;
    }
}

void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("[MQTT] Msg no tópico: %s\n", topic);
}

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char msg[len + 1];
    memcpy(msg, data, len);
    msg[len] = 0;
    printf("[MQTT] Payload: %s\n", msg);

    if (strstr(msg, "\"led\":\"ON\"")) {
        gpio_put(LED_GPIO, 1);
    } else if (strstr(msg, "\"led\":\"OFF\"")) {
        gpio_put(LED_GPIO, 0);
    }

    ssd1306_clear();
    ssd1306_draw_string(0, 0, "Recebido:");
    ssd1306_draw_string(0, 10, msg);
    ssd1306_draw_string(0, 30, "LED:");
    ssd1306_draw_string(40, 30, gpio_get(LED_GPIO) ? "ON" : "OFF");
    ssd1306_show();
}

void publish_button_state(bool pressed) {
    if (!mqtt_connected) return;

    char payload[64];
    snprintf(payload, sizeof(payload), "{\"botao\":\"%s\"}", pressed ? "ON" : "OFF");

    err_t err = mqtt_publish(mqtt_client, MQTT_TOPIC_PUB, payload, strlen(payload), 0, 0, NULL, NULL);
    if (err == ERR_OK) {
        printf("[MQTT] Publicado: %s\n", payload);
    } else {
        printf("[MQTT] Erro publicar: %d\n", err);
    }
}
