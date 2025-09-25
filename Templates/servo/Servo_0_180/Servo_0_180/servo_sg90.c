#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define SERVO_PIN   2   // GP2 -> sinal do servo
#define BTN_A_PIN   5   // GP5 -> Botão A (ativo em GND)
#define BTN_B_PIN   6   // GP6 -> Botão B (ativo em GND)

#define SERVO_FREQ_HZ   50          // 50 Hz -> 20 ms
#define SERVO_MIN_US    500.0f      // ~0°
#define SERVO_MAX_US   2500.0f      // ~180°

#define DEBOUNCE_MS     200         // tempo para ignorar repetições

static volatile float g_angle_deg = 90.0f; // inicia centralizado
static volatile uint32_t last_press_ms_A = 0;
static volatile uint32_t last_press_ms_B = 0;

// Inicializa PWM no pino do servo
static inline void servo_pwm_init(void) {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 125.0f);      // 125 MHz / 125 = 1 MHz (1 tick = 1 µs)
    pwm_config_set_wrap(&cfg, 20000 - 1);     // 20 ms
    pwm_init(slice, &cfg, true);
}

// Ajusta ângulo do servo (0 a 180 graus)
static inline void servo_set_angle(float deg) {
    if (deg < 0.0f)   deg = 0.0f;
    if (deg > 180.0f) deg = 180.0f;

    float us = SERVO_MIN_US + (deg / 180.0f) * (SERVO_MAX_US - SERVO_MIN_US);

    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    uint chan  = pwm_gpio_to_channel(SERVO_PIN);
    pwm_set_chan_level(slice, chan, (uint16_t)(us));
}

// Trata interrupções dos botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t now_ms = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN_A_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        if (now_ms - last_press_ms_A > DEBOUNCE_MS) {
            g_angle_deg = 0.0f;
            servo_set_angle(g_angle_deg);
            last_press_ms_A = now_ms;
        }
    }
    else if (gpio == BTN_B_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        if (now_ms - last_press_ms_B > DEBOUNCE_MS) {
            g_angle_deg = 180.0f;
            servo_set_angle(g_angle_deg);
            last_press_ms_B = now_ms;
        }
    }
}

// Configura pinos e interrupções
static void buttons_init(void) {
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true);
}

int main() {
    stdio_init_all();

    servo_pwm_init();
    buttons_init();

    // Posição inicial = 90°
    servo_set_angle(g_angle_deg);

    while (true) {
        tight_loop_contents();
        sleep_ms(1);
    }
}
