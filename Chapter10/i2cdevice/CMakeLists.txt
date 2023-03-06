cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(i2cdevice_project)

pico_sdk_init()

add_executable(i2cdevice
    i2cdevice.c
)

target_link_libraries(i2cdevice PRIVATE
    pico_stdlib
    pico_multicore
    pico_sync
    pico_i2c_slave
    hardware_i2c
    hardware_rtc
)

pico_enable_stdio_usb(i2cdevice 1)
pico_enable_stdio_uart(i2cdevice 0)

pico_add_extra_outputs(i2cdevice)

