#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "gps.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define GPS_TX_PIN 4  // Não usado (apenas recepção)
#define GPS_RX_PIN 5  // Conecte o TX do GPS aqui

char nmea_sentence[128];

int main() {
    stdio_init_all();
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(GPS_TX_PIN, GPIO_FUNC_UART);  // Mesmo que não usemos TX

    GPSData gps_data;

    printf("Iniciando leitura do GPS...\n");

    while (true) {
        int idx = 0;
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);

            if (c == '\n' && idx > 6) {
                nmea_sentence[idx] = '\0';

                if (parse_gpgga(nmea_sentence, &gps_data)) {
                    printf("Lat: %.6f, Lon: %.6f\n", gps_data.latitude, gps_data.longitude);
                }

                idx = 0;
            } else if (idx < sizeof(nmea_sentence) - 1) {
                nmea_sentence[idx++] = c;
            }
        }
    }
}
