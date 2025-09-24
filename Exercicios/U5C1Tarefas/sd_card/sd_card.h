#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdbool.h>
#include "ff.h"     // Biblioteca FATFS
#include "pico/stdlib.h"

// Configurações padrão de pinos SPI para Pico W
#define SD_SPI_PORT spi1
#define SD_PIN_MISO 12
#define SD_PIN_MOSI 11
#define SD_PIN_SCK  10
#define SD_PIN_CS   13

// Estrutura de controle do cartão SD
typedef struct {
    FATFS fs;       // Sistema de arquivos
    FIL file;       // Arquivo atual
    bool mounted;   // Status de montagem
} sd_card_t;

// Funções principais
bool sd_init(sd_card_t *sd);
bool sd_open_file(sd_card_t *sd, const char *filename, BYTE mode);
bool sd_write_file(sd_card_t *sd, const char *text);
bool sd_read_file(sd_card_t *sd, char *buffer, UINT bufsize);
void sd_close_file(sd_card_t *sd);
void sd_deinit(sd_card_t *sd);

#endif // SD_CARD_H
