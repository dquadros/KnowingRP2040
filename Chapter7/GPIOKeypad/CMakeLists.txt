cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(gpiokeypad_project)

pico_sdk_init()

add_executable(gpiokeypad
    gpiokeypad.c
)


target_link_libraries(gpiokeypad PRIVATE
    pico_stdlib
    hardware_sync
    hardware_gpio
)

pico_enable_stdio_usb(gpiokeypad 1)
pico_enable_stdio_uart(gpiokeypad 0)

pico_add_extra_outputs(gpiokeypad)
