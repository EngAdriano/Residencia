# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")


cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(pico_http_monitor C CXX ASM)

pico_sdk_init()

add_executable(pico_http_monitor
    main.c
)

target_link_libraries(pico_http_monitor
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
)

pico_enable_stdio_usb(pico_http_monitor 1)
pico_enable_stdio_uart(pico_http_monitor 1)

pico_add_extra_outputs(pico_http_monitor)
