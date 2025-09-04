#ifndef SERVO_CONT_H
#define SERVO_CONT_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"

typedef struct {
    uint gpio;
    uint slice;
    uint channel;
    uint16_t top;
    float stop_pulse_us;   // Pulso neutro (parado, ~1500us)
    float speed_scale;     // Quanto 1ms desvia da posição neutra
} servo_cont_t;

void servo_cont_init(servo_cont_t *servo, uint gpio);
void servo_cont_set_speed(servo_cont_t *servo, float speed); // -1.0 = máx esquerda, 0 = parar, +1.0 = máx direita
void servo_cont_move_time(servo_cont_t *servo, float speed, uint32_t ms);

#endif
