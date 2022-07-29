cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(i2c24c32_project)

pico_sdk_init()

add_executable(i2c24c32
    i2c24c32.c
)

target_link_libraries(i2c24c32 PRIVATE
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(i2c24c32 1)
pico_enable_stdio_uart(i2c24c32 0)

pico_add_extra_outputs(i2c24c32)

