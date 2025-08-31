#ifndef SERVO_H
#define SERVO_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"


typedef struct {
    uint gpio;           // Pino usado no PWM
    uint slice;          // Slice do PWM
    uint channel;        // Canal do PWM
    uint16_t top;        // Valor máximo (para 20 ms = 20000)
    float min_pulse_us;  // Pulso mínimo (ex: 500 us)
    float max_pulse_us;  // Pulso máximo (ex: 2500 us)
} servo_t;

void servo_init(servo_t *servo, uint gpio, float min_pulse_us, float max_pulse_us);
void servo_set_angle(servo_t *servo, float angle);

#endif
