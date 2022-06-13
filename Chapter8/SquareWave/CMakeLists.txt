cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(squarewave_project)

pico_sdk_init()

add_executable(squarewave
    squarewave.c
)

pico_generate_pio_header(squarewave ${CMAKE_CURRENT_LIST_DIR}/squarewave.pio)


target_link_libraries(squarewave PRIVATE
    pico_stdlib
    hardware_pio
)

pico_enable_stdio_usb(squarewave 1)
pico_enable_stdio_uart(squarewave 0)

pico_add_extra_outputs(squarewave)

