#include "servo_cont.h"

void servo_cont_init(servo_cont_t *servo, uint gpio) {
    servo->gpio = gpio;
    servo->stop_pulse_us = 1500.0f; 
    servo->speed_scale = 500.0f; // ±500us → ±velocidade

    gpio_set_function(gpio, GPIO_FUNC_PWM);
    servo->slice = pwm_gpio_to_slice_num(gpio);
    servo->channel = pwm_gpio_to_channel(gpio);

    float clk_div = 125.0f; // 125MHz / 125 = 1MHz → 1us/tick
    servo->top = 20000;     // 20ms período (50Hz)

    pwm_set_clkdiv(servo->slice, clk_div);
    pwm_set_wrap(servo->slice, servo->top);
    pwm_set_enabled(servo->slice, true);
}

void servo_cont_set_speed(servo_cont_t *servo, float speed) {
    if (speed > 1.0f) speed = 1.0f;
    if (speed < -1.0f) speed = -1.0f;

    float pulse_us = servo->stop_pulse_us + speed * servo->speed_scale;
    uint16_t level = (uint16_t)((pulse_us / 20000.0f) * servo->top);
    pwm_set_chan_level(servo->slice, servo->channel, level);
}

void servo_cont_move_time(servo_cont_t *servo, float speed, uint32_t ms) {
    servo_cont_set_speed(servo, speed);
    sleep_ms(ms);
    servo_cont_set_speed(servo, 0.0f); // parar
}
