cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(kbdhost_project)

pico_sdk_init()

add_executable(kbdhost
    kbdhost.c
)

target_include_directories(kbdhost PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(kbdhost PRIVATE
    pico_stdlib
    hardware_gpio
    hardware_uart
    tinyusb_host
    tinyusb_board
)

pico_add_extra_outputs(kbdhost)

