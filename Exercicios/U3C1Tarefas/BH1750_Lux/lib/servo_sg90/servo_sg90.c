#include "servo_sg90.h"

#define PWM_CLOCK 125000000.0f   // 125 MHz clock base
#define SERVO_FREQ 50.0f         // 50 Hz -> período de 20ms
#define SERVO_PERIOD_US 20000.0f // 20.000 µs

void servo_init(servo_t *servo, uint gpio, float min_pulse_us, float max_pulse_us) {
    servo->gpio = gpio;
    servo->min_pulse_us = min_pulse_us;
    servo->max_pulse_us = max_pulse_us;

    gpio_set_function(gpio, GPIO_FUNC_PWM);
    servo->slice = pwm_gpio_to_slice_num(gpio);
    servo->channel = pwm_gpio_to_channel(gpio);

    pwm_config config = pwm_get_default_config();

    // Divisor para atingir wrap razoável em 50 Hz
    float clkdiv = 125.0f; // 125MHz / 125 = 1MHz -> tick = 1µs
    uint32_t top = (uint32_t)(SERVO_PERIOD_US - 1); // 20000-1 = 19999

    pwm_config_set_clkdiv(&config, clkdiv);
    pwm_config_set_wrap(&config, top);

    servo->top = top;

    pwm_init(servo->slice, &config, true);
}

void servo_set_pulse_us(servo_t *servo, float pulse_us) {
    if (pulse_us < servo->min_pulse_us) pulse_us = servo->min_pulse_us;
    if (pulse_us > servo->max_pulse_us) pulse_us = servo->max_pulse_us;

    uint16_t level = (uint16_t)((pulse_us / SERVO_PERIOD_US) * (servo->top + 1));
    pwm_set_chan_level(servo->slice, servo->channel, level);
}

void servo_set_angle(servo_t *servo, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_us = servo->min_pulse_us +
                    (angle / 180.0f) * (servo->max_pulse_us - servo->min_pulse_us);

    servo_set_pulse_us(servo, pulse_us);
}
