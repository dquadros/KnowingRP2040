cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pioint_project)

pico_sdk_init()

add_executable(pioint
    pioint.c
)

pico_generate_pio_header(pioint ${CMAKE_CURRENT_LIST_DIR}/pioint.pio)

target_link_libraries(pioint PRIVATE
    pico_stdlib
    pico_sync
    hardware_pio
    hardware_irq
)

pico_enable_stdio_usb(pioint 1)
pico_enable_stdio_uart(pioint 0)

pico_add_extra_outputs(pioint)

