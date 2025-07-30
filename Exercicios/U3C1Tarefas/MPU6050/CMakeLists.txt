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

project(mpu6050_project)

pico_sdk_init()

add_executable(mpu6050
    mpu6050_sdk.cpp
    lib/mpu6050/mpu6050.cpp
    lib/ssd1306/ssd1306.c
)

# Add the standard include files to the build
target_include_directories(mpu6050 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/mpu6050
        ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306
)

target_link_libraries(mpu6050 PRIVATE
    pico_stdlib
    hardware_gpio
    hardware_i2c
)

pico_enable_stdio_usb(mpu6050 1)
pico_enable_stdio_uart(mpu6050 0)

pico_add_extra_outputs(mpu6050)
