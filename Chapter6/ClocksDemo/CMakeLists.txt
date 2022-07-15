cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(clocksdemo_project)

pico_sdk_init()

add_executable(clocksdemo
        clocksdemo.c
        )

target_link_libraries(clocksdemo PRIVATE
	pico_stdlib 
	hardware_clocks)

pico_enable_stdio_usb(clocksdemo 1)
pico_enable_stdio_uart(clocksdemo 0)

pico_add_extra_outputs(clocksdemo)