cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(gpio_interrupt_project)

pico_sdk_init()

add_executable(gpio_interrupt
    gpiointerrupt.c
)

target_link_libraries(gpio_interrupt PRIVATE
    pico_stdlib
    hardware_gpio
)

pico_enable_stdio_usb(gpio_interrupt 1)
pico_enable_stdio_uart(gpio_interrupt 0)

pico_add_extra_outputs(gpio_interrupt)
