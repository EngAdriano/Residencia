#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "html_page.h"

#define SSID "Lu e Deza"
#define PASSWORD "liukin1208"
#define GPIO_BUTTON 5
#define PORT 80

int gpio_status = 0;

// Handler de requisições
err_t http_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *pbuffer, err_t err) {
    if (!pbuffer) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    tcp_recved(tpcb, pbuffer->tot_len);

    // Verifica se a requisição é válida
    char *req = calloc(pbuffer->len + 1, 1);
    memcpy(req, pbuffer->payload, pbuffer->len);
    req[pbuffer->len] = '\0';

    printf("Requisição recebida:\n%s\n", req);

    // Verifica se a requisição é do tipo GET
    if (strstr(req, "GET /status")) {
        char resp[64];
        const char *estado = gpio_status ? "PRESSIONADO" : "LIBERADO";

        printf("Status do botão: %s\n", estado);

        sprintf(resp, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n%s", estado);
        tcp_write(tpcb, resp, strlen(resp), TCP_WRITE_FLAG_COPY);
    } else {
        printf("Enviando página HTML inicial\n");
        tcp_write(tpcb, html_page, strlen(html_page), TCP_WRITE_FLAG_COPY);
    }

    tcp_output(tpcb);        // envia os dados
    tcp_close(tpcb);         // fecha a conexão corretamente
    free(req);               // libera memória alocada
    pbuf_free(pbuffer);      // libera o buffer de recepção  
    return ERR_OK;
}


// Handler de conexão
// Recebe conexões TCP
err_t http_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_server_recv);
    return ERR_OK;
}

// Inicializa o servidor
// Cria um PCB TCP, escuta na porta 80 e aceita conexões
// PCB (Protocol Control Block) é uma estrutura de dados usada pelo LwIP para gerenciar conexões TCP
void start_server() {
    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_server_accept);
}

// Inicializa o Wi-Fi e o servidor
// Configura o Wi-Fi em modo STA e conecta à rede
int main() {
    stdio_init_all();  // Inicializa porta serial via USB

    gpio_init(GPIO_BUTTON);
    gpio_set_dir(GPIO_BUTTON, GPIO_IN);
    gpio_pull_up(GPIO_BUTTON); // botão entre GPIO e GND

    if (cyw43_arch_init()) {
        printf("Falha ao iniciar Wi-Fi\n");
        return 1;
    }

    // Configura o Wi-Fi em modo STA
    cyw43_arch_enable_sta_mode();
    
    if (cyw43_arch_wifi_connect_timeout_ms(SSID, PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha na conexão Wi-Fi\n");
        return 1;
    }
    
    // Aguarda o IP ser atribuído
    printf("Conectado à rede. IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));

    // Inicializa o servidor
    start_server();

    // Loop principal
    // Verifica o estado do botão a cada 100ms
    while (true) {
        gpio_status = !gpio_get(GPIO_BUTTON); // Inverte se pull-up
        sleep_ms(100);
    }
}
