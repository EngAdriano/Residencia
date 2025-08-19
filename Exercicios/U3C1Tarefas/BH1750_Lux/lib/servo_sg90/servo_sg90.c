#include "servo_sg90.h"

void servo_init(servo_t *servo, uint gpio, float min_pulse_us, float max_pulse_us) {
    servo->gpio = gpio;
    servo->min_pulse_us = min_pulse_us;
    servo->max_pulse_us = max_pulse_us;

    gpio_set_function(gpio, GPIO_FUNC_PWM);

    servo->slice = pwm_gpio_to_slice_num(gpio);
    servo->channel = pwm_gpio_to_channel(gpio);

    // Frequência alvo = 50Hz → período = 20ms
    // clock padrão = 125 MHz
    float clk_div = 125.0f;
    servo->top = 20000; // 20ms com clock dividido por 125 → 1 tick = 1us

    pwm_set_clkdiv(servo->slice, clk_div);
    pwm_set_wrap(servo->slice, servo->top);
    pwm_set_enabled(servo->slice, true);
}

void servo_set_angle(servo_t *servo, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_us = servo->min_pulse_us +
                     (angle / 180.0f) * (servo->max_pulse_us - servo->min_pulse_us);

    uint16_t level = (uint16_t)((pulse_us / 20000.0f) * servo->top);
    pwm_set_chan_level(servo->slice, servo->channel, level);
}
