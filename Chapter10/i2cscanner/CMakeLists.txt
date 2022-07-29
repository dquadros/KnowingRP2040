cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(i2cscanner_project)

pico_sdk_init()

add_executable(i2cscanner
    i2cscanner.c
)

target_link_libraries(i2cscanner PRIVATE
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(i2cscanner 1)
pico_enable_stdio_uart(i2cscanner 0)

pico_add_extra_outputs(i2cscanner)

