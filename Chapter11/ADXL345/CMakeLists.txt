cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(adxl345_project)

pico_sdk_init()

add_executable(adxl345
    adxl345.c
)

target_link_libraries(adxl345 PRIVATE
    pico_stdlib
    hardware_spi
)

pico_enable_stdio_usb(adxl345 1)
pico_enable_stdio_uart(adxl345 0)

pico_add_extra_outputs(adxl345)

