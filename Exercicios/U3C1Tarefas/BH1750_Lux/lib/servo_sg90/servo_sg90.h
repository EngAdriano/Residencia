#ifndef SERVO_H
#define SERVO_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"

typedef struct {
    uint gpio;
    uint slice;
    uint channel;
    uint16_t top;
    float min_pulse_us;
    float max_pulse_us;
} servo_t;

void servo_init(servo_t *servo, uint gpio, float min_pulse_us, float max_pulse_us);
void servo_set_angle(servo_t *servo, float angle);

#endif
