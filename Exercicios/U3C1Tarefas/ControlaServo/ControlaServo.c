#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 18

int main() {
    stdio_init_all();

    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    uint channel = pwm_gpio_to_channel(SERVO_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);    // 1 tick = 1 µs
    pwm_config_set_wrap(&config, 20000 - 1);   // 20 ms = 50 Hz
    pwm_init(slice, &config, true);

    while (true) {
        pwm_set_chan_level(slice, channel, 500);   // 0°
        sleep_ms(2000);

        pwm_set_chan_level(slice, channel, 1500);  // 90°
        sleep_ms(2000);

        pwm_set_chan_level(slice, channel, 2500);  // 180°
        sleep_ms(2000);
    }
}
