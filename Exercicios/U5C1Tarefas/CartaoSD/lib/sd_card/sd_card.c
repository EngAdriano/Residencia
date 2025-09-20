#include "sd_card.h"
#include "hw_config.h"   // Necessário para ligação com FatFS e pico-sdk
#include "ff.h"
#include "diskio.h"
#include <string.h>

// Inicializa o cartão SD e monta o sistema de arquivos
bool sd_init(sd_card_t *sd) {
    FRESULT fr;
    if (!sd) return false;

    // Monta o sistema de arquivos
    fr = f_mount(&sd->fs, "", 1);
    if (fr != FR_OK) {
        sd->mounted = false;
        return false;
    }
    sd->mounted = true;
    return true;
}

// Abre arquivo
bool sd_open_file(sd_card_t *sd, const char *filename, BYTE mode) {
    if (!sd || !sd->mounted) return false;
    FRESULT fr = f_open(&sd->file, filename, mode);
    return (fr == FR_OK);
}

// Escreve texto no arquivo
bool sd_write_file(sd_card_t *sd, const char *text) {
    if (!sd || !sd->mounted) return false;
    UINT bw;
    FRESULT fr = f_write(&sd->file, text, strlen(text), &bw);
    return (fr == FR_OK && bw == strlen(text));
}

// Lê conteúdo do arquivo
bool sd_read_file(sd_card_t *sd, char *buffer, UINT bufsize) {
    if (!sd || !sd->mounted) return false;
    UINT br;
    FRESULT fr = f_read(&sd->file, buffer, bufsize - 1, &br);
    if (fr == FR_OK) {
        buffer[br] = '\0'; // Garante string terminada
        return true;
    }
    return false;
}

// Fecha arquivo
void sd_close_file(sd_card_t *sd) {
    if (!sd) return;
    f_close(&sd->file);
}

// Desmonta SD
void sd_deinit(sd_card_t *sd) {
    if (!sd) return;
    f_mount(NULL, "", 0);
    sd->mounted = false;
}
