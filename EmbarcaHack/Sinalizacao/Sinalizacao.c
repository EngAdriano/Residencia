/*
// Projeto: Sinalização de emergência para parque eólico
// EmbarcaHack
// Equipe: Felipe Teles, José Adriano e Flávio Dias
//
// Projeto para sinalizar em parques eólicos a próximidade de
// descargas elétricas, possibilitando ao pessoal operacional
// receber avisos de emergências e assim se resguardarem de 
// possíveis acidentes.
//
// Data: 20/05/2025
// Esta versão é uma Prova de conceito (POC), caso seja aceito,
// a implementação será com acesso a API's de servidores atmosféricos 
// afim de automatizar a tomada de decisão para acionar a sinalização.
*/


#include <stdio.h>         // Biblioteca padrão de entrada e saída
#include "hardware/adc.h"  // Biblioteca para manipulação do ADC (Conversor Analógico-Digital)
#include "hardware/gpio.h" // Biblioteca para controle dos pinos GPIO
#include "pico/stdlib.h"   // Biblioteca padrão do Raspberry Pi Pico

// Definição dos pinos conectados ao joystick e dispositivos de saída
#define VRX 26            // Pino do eixo X do joystick (conectado ao ADC)
#define VRY 27            // Pino do eixo Y do joystick (conectado ao ADC)
#define SW 22             // Pino do botão do joystick (entrada digital)
#define LED_VERMELHO 14   // Pino para o LED vermelho
#define LED_AMARELO 15    // Pino para o LED amarelo
//#define LED_VERMELHO 13   // Pino para o LED vermelho
//#define LED_AMARELO 12    // Pino para o LED amarelo
#define BUZZER 10        // Pino para o buzzer

// Definição dos canais ADC correspondentes aos pinos VRX e VRY
#define ADC_CHANNEL_X 0   // Canal do ADC para eixo X
#define ADC_CHANNEL_Y 1   // Canal do ADC para eixo Y

// Definição dos limites para a posição central e margem de tolerância
#define CENTRO 2048       // Valor médio esperado dos ADCs (12 bits, faixa de 0 a 4095)
#define MARGEM 500        // Margem de tolerância para detectar movimentos

// Função para configurar os pinos e módulos necessários
void setup() {
    stdio_init_all();   // Inicializa a comunicação serial (não usada neste código)
    
    adc_init();         // Inicializa o módulo ADC para leitura analógica
    adc_gpio_init(VRX); // Configura o pino VRX como entrada de ADC
    adc_gpio_init(VRY); // Configura o pino VRY como entrada de ADC

    gpio_init(SW);             // Inicializa o pino do botão do joystick
    gpio_set_dir(SW, GPIO_IN); // Configura o pino como entrada digital
    gpio_pull_up(SW);          // Ativa o pull-up interno para evitar leituras flutuantes

    // Configuração dos pinos dos LEDs e do buzzer como saída
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    
    gpio_init(LED_AMARELO);
    gpio_set_dir(LED_AMARELO, GPIO_OUT);
    
    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
}

// Função para ler um valor do ADC (Analógico-Digital Converter)
uint16_t ler_adc(uint channel) {
    adc_select_input(channel); // Seleciona o canal correto do ADC
    return adc_read();         // Retorna o valor lido (entre 0 e 4095)
}

// Função para piscar um LED
void piscar_led(uint pin) {
    gpio_put(pin, 1);    // Liga o LED
    sleep_ms(100);       // Espera 100ms
    gpio_put(pin, 0);    // Desliga o LED
}

int main() {
    setup(); // Configuração inicial dos pinos e ADC

    while (true) {
        // Lê os valores do joystick (eixo X e eixo Y)
        uint16_t x = ler_adc(ADC_CHANNEL_X);
        uint16_t y = ler_adc(ADC_CHANNEL_Y);
        bool sw = gpio_get(SW); // Lê o estado do botão do joystick

        // Desativa todos os dispositivos antes de analisar os movimentos
        gpio_put(LED_VERMELHO, 0);
        gpio_put(LED_AMARELO, 0);
        gpio_put(BUZZER, 0);

        // Lógica de acionamento com base na posição do joystick
        if (x > CENTRO + MARGEM) {
            // Joystick puxado para a direita
            gpio_put(LED_VERMELHO, 1);
            gpio_put(LED_AMARELO, 1);
            gpio_put(BUZZER, 1);
        } else if (x < CENTRO - MARGEM) {
            // Joystick puxado para a esquerda
            gpio_put(LED_VERMELHO, 1);
        } else if (y > CENTRO + MARGEM) {
            // Joystick puxado para cima
            piscar_led(LED_VERMELHO);
        } else if (y < CENTRO - MARGEM) {
            // Joystick puxado para baixo
            gpio_put(LED_AMARELO, 1);
        }

        sleep_ms(100); // Pequena pausa antes da próxima leitura
    }
}
