cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(sleep_project)

pico_sdk_init()

add_executable(sleep
        sleep.c
        )

target_link_libraries(sleep 
	pico_stdlib
	pico_time 
	hardware_sleep
	)

pico_add_extra_outputs(sleep)

