cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(serialrx_project)

pico_sdk_init()

add_executable(serialrx
    serialrx.c
)

pico_generate_pio_header(serialrx ${CMAKE_CURRENT_LIST_DIR}/serialrx.pio)


target_link_libraries(serialrx PRIVATE
    pico_stdlib
    pico_stdio
    hardware_pio
)

pico_enable_stdio_usb(serialrx 1)
pico_enable_stdio_uart(serialrx 0)

pico_add_extra_outputs(serialrx)

