cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(uartsum_project)

pico_sdk_init()

add_executable(uartsum
    uartsum.c
)

target_link_libraries(uartsum PRIVATE
    pico_stdlib
    hardware_uart
)

pico_enable_stdio_usb(uartsum 1)
pico_enable_stdio_uart(uartsum 0)

pico_add_extra_outputs(uartsum)

