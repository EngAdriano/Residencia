#include "servo_sg90.h"

void servo_init(servo_t *servo, uint gpio, float min_pulse_us, float max_pulse_us) {
    servo->gpio = gpio;
    servo->min_pulse_us = min_pulse_us;
    servo->max_pulse_us = max_pulse_us;

    gpio_set_function(gpio, GPIO_FUNC_PWM);

    servo->slice = pwm_gpio_to_slice_num(gpio);
    servo->channel = pwm_gpio_to_channel(gpio);

    pwm_config config = pwm_get_default_config();

    // Queremos clock de 1 MHz → 1 tick = 1 us
    pwm_config_set_clkdiv(&config, 125.0f);

    // Período de 20 ms = 20000 ticks
    pwm_config_set_wrap(&config, 20000 - 1);
    servo->top = 20000 - 1;

    pwm_init(servo->slice, &config, true);
}

void servo_set_angle(servo_t *servo, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    float pulse_us = servo->min_pulse_us +
                     (angle / 180.0f) * (servo->max_pulse_us - servo->min_pulse_us);

    // Agora mandamos direto o tempo em µs
    pwm_set_chan_level(servo->slice, servo->channel, (uint16_t)pulse_us);
}
