cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(gpio7segment_project)

pico_sdk_init()

add_executable(gpio7segment
    gpio7segment.c
)


target_link_libraries(gpio7segment PRIVATE
    pico_stdlib
    hardware_sync
    hardware_gpio
)

pico_add_extra_outputs(gpio7segment)
