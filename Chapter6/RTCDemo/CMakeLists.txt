cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(rtcdemo_project)

pico_sdk_init()

add_executable(rtcdemo
        rtcdemo.c
        )

target_link_libraries(rtcdemo PRIVATE
	pico_stdlib 
	hardware_rtc)

pico_enable_stdio_usb(rtcdemo 1)
pico_enable_stdio_uart(rtcdemo 0)

pico_add_extra_outputs(rtcdemo)