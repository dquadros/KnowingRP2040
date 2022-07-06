cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(adcdemo_project)

pico_sdk_init()

add_executable(adcdemo
    adcdemo.c
)


target_link_libraries(adcdemo PRIVATE
    pico_stdlib
    hardware_adc
    hardware_gpio
)

pico_enable_stdio_usb(adcdemo 1)
pico_enable_stdio_uart(adcdemo 0)

pico_add_extra_outputs(adcdemo)

