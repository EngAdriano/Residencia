#include <stdio.h>
#include "pico/stdlib.h"
#include "mpu6050/mpu6050.h"
#include "servo/servo.h"
#include "display/display.h"

#define ANGLE_ALERT_THRESHOLD 60.0f // Ângulo limite para alerta
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180

int main()
{
    // Inicializando dispositivos
    stdio_init_all();
    mpu6050_init();
    servo_init();
    display_init();

    int16_t ax, ay, az;
    float angle;      // Angulo medido pelo acelerometro
    uint angle_servo; // Angulo a aplicar no servo motor
    char buffer[64];

    while (true) {
        mpu6050_read_raw(&ax, &ay, &az);
        angle = mpu6050_get_inclination(ax, ay, az);

        // Mapeia o ângulo de inclinação para o range do servo (0-180)
        angle_servo = (uint)((angle + 90.0f) * (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 180.0f);
        if (angle_servo > SERVO_MAX_ANGLE) angle_servo = SERVO_MAX_ANGLE;
        servo_set_angle(angle_servo);

        // Exibe dados no monitor serial
        printf("AX: %d | AY: %d | AZ: %d | Angulo: %.2f | Servo: %u\n", ax, ay, az, angle, angle_servo);

        // Exibe ângulo no display
        snprintf(buffer, sizeof(buffer), "Ang.:%.1f", angle);
        display_text(buffer, 0, 24, 2);

        // Alerta visual se ultrapassar o limite
        if (fabs(angle) > ANGLE_ALERT_THRESHOLD) {
            display_text_no_clear("ALERTA!", 20, 0, 2);
        }
        show_display();

        sleep_ms(150);
    }
}

