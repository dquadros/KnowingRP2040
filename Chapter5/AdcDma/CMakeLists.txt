cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(adcdma_project)

pico_sdk_init()

add_executable(adcdma
    adcdma.c
)


target_link_libraries(adcdma PRIVATE
    pico_stdlib
    hardware_adc
    hardware_dma
)

pico_enable_stdio_usb(adcdma 0)
pico_enable_stdio_uart(adcdma 1)

pico_add_extra_outputs(adcdma)

