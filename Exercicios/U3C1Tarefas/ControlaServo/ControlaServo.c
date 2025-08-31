#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 18

void servo_set_angle(uint slice, uint channel, float angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Pulso entre 500 e 2500 µs
    uint16_t level = 500 + (uint16_t)((angle / 180.0f) * 2000);
    pwm_set_chan_level(slice, channel, level);
}

int main() {
    stdio_init_all();

    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    uint channel = pwm_gpio_to_channel(SERVO_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);   // 1 tick = 1 µs
    pwm_config_set_wrap(&config, 20000 - 1);  // 20 ms
    pwm_init(slice, &config, true);

    // Envia 90° e mantém
    servo_set_angle(slice, channel, 180);
    while (true) {
        sleep_ms(1000);
    }
}
