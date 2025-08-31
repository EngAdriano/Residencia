#include "pico/stdlib.h"
#include "servo_velocity.h"

int main() {
    stdio_init_all();

    servo_velocity_t servo;
    servo_init(&servo, 18, 0.02f);  // GPIO18, ganho Kp=0.02

    uint32_t counter = 0;
    float angles[] = {0, 30, 60, 90, 120, 150, 180};
    int idx = 0;

    while (true) {
        servo_update(&servo); // atualiza servo continuamente

        // Rotina de teste: altera alvo a cada 3s
        // Observei que o ponto 0 é onde ele se encontra quando desligado.
        sleep_ms(20);
        counter += 20;
        if (counter >= 3000) {
            counter = 0;
            idx++;
            if (idx >= 6) idx = 0;
            servo_set_target_angle(&servo, angles[idx]);
        }
    }
}
