cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(watchdogdemo_project)

pico_sdk_init()

add_executable(watchdogdemo
        watchdogdemo.c
        )

target_link_libraries(watchdogdemo PRIVATE
	pico_stdlib 
	hardware_watchdog)

pico_enable_stdio_usb(watchdogdemo 1)
pico_enable_stdio_uart(watchdogdemo 0)

pico_add_extra_outputs(watchdogdemo)