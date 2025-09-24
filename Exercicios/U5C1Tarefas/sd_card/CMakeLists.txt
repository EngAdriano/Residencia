# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(sd_card C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add FatFs library
add_subdirectory(FatFs_SPI build)

# Add executable. Default name is the project name, version 0.1

add_executable(sd_card 
        sd_card.c 
        hw_config.c
)

pico_set_program_name(sd_card "sd_card")
pico_set_program_version(sd_card "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(sd_card 0)
pico_enable_stdio_usb(sd_card 1)

# Add the standard library to the build
target_link_libraries(sd_card
        pico_stdlib
        FatFs_SPI
)

# Add the standard include files to the build
target_include_directories(sd_card PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(sd_card 
        
        )

pico_add_extra_outputs(sd_card)

