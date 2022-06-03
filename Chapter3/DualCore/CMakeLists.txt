cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(dualcore_project)

pico_sdk_init()

add_executable(dualcore
    dualcore.c
)


target_link_libraries(dualcore PRIVATE
    pico_stdlib
    pico_multicore
    pico_sync
    hardware_adc
    hardware_gpio
)

pico_enable_stdio_usb(dualcore 1)
pico_enable_stdio_uart(dualcore 0)

pico_add_extra_outputs(dualcore)


