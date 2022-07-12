cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pwmmeasure_project)

pico_sdk_init()

add_executable(pwmmeasure
    pwmmeasure.c
)


target_link_libraries(pwmmeasure PRIVATE
    pico_stdlib
    hardware_pwm
    hardware_gpio
    hardware_clocks
)

pico_enable_stdio_usb(pwmmeasure 1)
pico_enable_stdio_uart(pwmmeasure 0)

pico_add_extra_outputs(pwmmeasure)

