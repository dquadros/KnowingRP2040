cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pwmdemo_project)

pico_sdk_init()

add_executable(pwmdemo
    pwmdemo.c
)


target_link_libraries(pwmdemo PRIVATE
    pico_stdlib
    hardware_pwm
    hardware_gpio
    hardware_clocks
)

pico_enable_stdio_usb(pwmdemo 1)
pico_enable_stdio_uart(pwmdemo 0)

pico_add_extra_outputs(pwmdemo)

