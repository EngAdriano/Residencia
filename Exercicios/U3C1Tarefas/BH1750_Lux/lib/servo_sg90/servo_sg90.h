#ifndef SERVO_NG90_H
#define SERVO_NG90_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"

typedef struct {
    uint gpio;             // GPIO usado
    uint slice;            // Slice do PWM
    uint channel;          // Canal do PWM (A ou B)
    uint16_t top;          // Valor de wrap (período do PWM)
    float min_pulse_us;    // Pulso mínimo (µs)
    float max_pulse_us;    // Pulso máximo (µs)
} servo_t;

/**
 * Inicializa o servo NG90 em um pino específico
 * @param servo ponteiro para struct servo_t
 * @param gpio número do GPIO do Pico
 * @param min_pulse_us pulso mínimo em microssegundos (ex: 500)
 * @param max_pulse_us pulso máximo em microssegundos (ex: 2500)
 */
void servo_init(servo_t *servo, uint gpio, float min_pulse_us, float max_pulse_us);

/**
 * Define o ângulo do servo em graus
 * @param servo ponteiro para struct servo_t
 * @param angle ângulo desejado (0° – 180°)
 */
void servo_set_angle(servo_t *servo, float angle);

/**
 * Define diretamente o pulso em microssegundos
 * @param servo ponteiro para struct servo_t
 * @param pulse_us largura do pulso em µs (ex: 1500 = ~90°)
 */
void servo_set_pulse_us(servo_t *servo, float pulse_us);

#endif
