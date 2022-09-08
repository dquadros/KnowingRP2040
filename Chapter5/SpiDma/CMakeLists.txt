cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(spidma_project)

pico_sdk_init()

add_executable(spidma
    spidma.c
)


target_link_libraries(spidma PRIVATE
    pico_stdlib
    hardware_spi
    hardware_dma
)

pico_enable_stdio_usb(spidma 1)
pico_enable_stdio_uart(spidma 0)

pico_add_extra_outputs(spidma)

