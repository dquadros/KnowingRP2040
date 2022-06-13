cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hcsr04_project)

pico_sdk_init()

add_executable(hcsr04
    hcsr04.c
)

pico_generate_pio_header(hcsr04 ${CMAKE_CURRENT_LIST_DIR}/hcsr04.pio)


target_link_libraries(hcsr04 PRIVATE
    pico_stdlib
    pico_stdio
    hardware_pio
)

pico_enable_stdio_usb(hcsr04 1)
pico_enable_stdio_uart(hcsr04 0)

pico_add_extra_outputs(hcsr04)

