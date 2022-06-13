cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(serialtx_project)

pico_sdk_init()

add_executable(serialtx
    serialtx.c
)

pico_generate_pio_header(serialtx ${CMAKE_CURRENT_LIST_DIR}/serialtx.pio)


target_link_libraries(serialtx PRIVATE
    pico_stdlib
    hardware_pio
)

pico_enable_stdio_usb(serialtx 1)
pico_enable_stdio_uart(serialtx 0)

pico_add_extra_outputs(serialtx)

