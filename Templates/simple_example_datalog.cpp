#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "ff.h"         // FatFs
#include "f_util.h"
#include "hw_config.h"  // sd_init_driver(), sd_get_by_num()

// ======= Configurações =======
#define NUM_REGISTROS        60      // quantas linhas escrever no txt
#define INTERVALO_MS         1000    // intervalo entre leituras (ms)
#define N_SAMPLES_MEDIA      16      // média de N leituras ADC por linha
#define ADC_REF_VOLT         3.3f    // tensão de referência (padrão)
#define ADC_MAX_COUNT        4095.0f // 12 bits (0..4095)

// Fórmula do datasheet (RP2040):
// Temp(°C) = 27 - (V_sense - 0.706)/0.001721
static float adc_to_celsius(uint16_t raw)
{
    float v = (raw * ADC_REF_VOLT) / ADC_MAX_COUNT;
    float temp_c = 27.0f - (v - 0.706f) / 0.001721f;
    return temp_c;
}

int main(void)
{
    stdio_init_all();
    sleep_ms(500); // opcional: dar tempo para USB conectar

    // -------- Inicializa ADC para sensor interno --------
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // canal 4 = sensor interno

    // -------- Inicializa e monta o cartão SD --------
    if (!sd_init_driver()) {
        panic("sd_init_driver() failed\n");
    }

    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    // Monta caminho "0:/temp_log.txt"
    FIL fil;
    char path[64];
    snprintf(path, sizeof(path), "%s/%s", pSD->pcName, "temp_log.txt");

    fr = f_open(&fil, path, FA_OPEN_ALWAYS | FA_WRITE);
    if (fr != FR_OK) {
        panic("f_open(%s) error: %s (%d)\n", path, FRESULT_str(fr), fr);
    }
    // Move o ponteiro para o fim (modo append manual)
    f_lseek(&fil, f_size(&fil));

    // Cabeçalho (escreve uma vez)
#if FF_USE_STRFUNC
    f_printf(&fil, "timestamp_ms,temperature_c\n");
#else
    {
        const char *hdr = "timestamp_ms,temperature_c\n";
        UINT bw = 0;
        fr = f_write(&fil, hdr, strlen(hdr), &bw);
        if (fr != FR_OK || bw != strlen(hdr)) {
            printf("f_write header failed: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }
#endif
    f_sync(&fil);

    // -------- Loop de leituras --------
    absolute_time_t t0 = get_absolute_time();
    for (int i = 0; i < NUM_REGISTROS; i++) {
        // Média de N_SAMPLES_MEDIA leituras para reduzir ruído
        uint32_t acc = 0;
        for (int k = 0; k < N_SAMPLES_MEDIA; k++) {
            uint16_t raw = adc_read();
            acc += raw;
            sleep_us(200); // pequeno intervalo entre amostras
        }
        uint16_t raw_avg = (uint16_t)(acc / N_SAMPLES_MEDIA);
        float temp_c = adc_to_celsius(raw_avg);

        // Timestamp relativo em ms (desde boot)
        uint32_t ts_ms = to_ms_since_boot(t0) + i * INTERVALO_MS;

        // Escreve linha CSV
#if FF_USE_STRFUNC
        if (f_printf(&fil, "%u,%.3f\n", ts_ms, temp_c) < 0) {
            printf("f_printf failed\n");
        }
#else
        {
            char line[64];
            int n = snprintf(line, sizeof(line), "%u,%.3f\n", ts_ms, temp_c);
            UINT bw = 0;
            fr = f_write(&fil, line, (UINT)n, &bw);
            if (fr != FR_OK || bw != (UINT)n) {
                printf("f_write failed: %s (%d), bw=%u\n", FRESULT_str(fr), fr, bw);
            }
        }
#endif

        // Garante flush periódico (útil se resetar no meio)
        f_sync(&fil);
        sleep_ms(INTERVALO_MS);
    }

    // -------- Fecha e desmonta --------
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    f_unmount(pSD->pcName);

    puts("Done. Lines written to temp_log.txt");
    while (true) {
        sleep_ms(1000);
    }
    // return 0; // nunca alcançado
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// hw_config.c

/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use 
this file except in compliance with the License. You may obtain a copy of the 
License at

   http://www.apache.org/licenses/LICENSE-2.0 
Unless required by applicable law or agreed to in writing, software distributed 
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
*/
/*

This file should be tailored to match the hardware design.

There should be one element of the spi[] array for each hardware SPI used.

There should be one element of the sd_cards[] array for each SD card slot.
The name is should correspond to the FatFs "logical drive" identifier.
(See http://elm-chan.org/fsw/ff/doc/filename.html#vol)
The rest of the constants will depend on the type of
socket, which SPI it is driven by, and how it is wired.

*/

#include <string.h>
//
#include "my_debug.h"
//
#include "hw_config.h"
//
#include "ff.h" /* Obtains integer types */
//
#include "diskio.h" /* Declarations of disk functions */

// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = spi0,  // SPI component
        .miso_gpio = 16, // GPIO number (not pin number)
        .mosi_gpio = 19,
        .sck_gpio = 18,
        .baud_rate = 12500 * 1000,  
        //.baud_rate = 25 * 1000 * 1000, // Actual frequency: 20833333. 
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",   // Name used to mount device
        .spi = &spis[0],  // Pointer to the SPI driving this card
        .ss_gpio = 9,    // The SPI slave select GPIO for this SD card
        .use_card_detect = true,
        .card_detect_gpio = 13,   // Card detect
        .card_detected_true = 1  // What the GPIO read returns when a card is
                                 // present. Use -1 if there is no card detect.
    }};

/* ********************************************************************** */
size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//cmakerlist
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
#set(PICO_SDK_PATH "/home/carlk/pi/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(simple_example C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(../FatFs_SPI build)

# Add executable. Default name is the project name, version 0.1
add_executable(simple_example 
    simple_example.cpp
    hw_config.c
)

# Add the standard library and FatFS/SPI to the build
target_link_libraries(simple_example 
    pico_stdlib
    FatFs_SPI
    hardware_adc
)

pico_set_program_name(simple_example "simple_example")
pico_set_program_version(simple_example "0.1")

# Choose source and destination for standard input and output:
#   See 4.1. Serial input and output on Raspberry Pi Pico in Getting started with Raspberry Pi Pico (https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
#   and 2.7.1. Standard Input/Output (stdio) Support in Raspberry Pi Pico C/C++ SDK (https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf):
pico_enable_stdio_uart(simple_example 1)
pico_enable_stdio_usb(simple_example 1)

pico_add_extra_outputs(simple_example)



