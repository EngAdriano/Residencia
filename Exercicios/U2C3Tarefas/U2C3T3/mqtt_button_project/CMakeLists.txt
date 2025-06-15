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

project(mqtt_button_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(mqtt_button_project
    main.c
    paho/MQTTClient.c
    paho/MQTTConnectClient.c
    paho/MQTTSubscribeClient.c
    paho/MQTTUnsubscribeClient.c
    paho/MQTTFormat.c
    paho/Network.c
    paho/NetworkTLS.c
)

target_include_directories(mqtt_button_project PRIVATE
    paho
)

target_link_libraries(mqtt_button_project
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_tls
)

pico_add_extra_outputs(mqtt_button_project)
