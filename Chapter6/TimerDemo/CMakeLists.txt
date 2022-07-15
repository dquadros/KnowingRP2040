cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(timerdemo_project)

pico_sdk_init()

add_executable(timerdemo
        timerdemo.c
        )

target_link_libraries(timerdemo PRIVATE
	pico_stdlib 
	hardware_timer)

pico_enable_stdio_usb(timerdemo 1)
pico_enable_stdio_uart(timerdemo 0)

pico_add_extra_outputs(timerdemo)