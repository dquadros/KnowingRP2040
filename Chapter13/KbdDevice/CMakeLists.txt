cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(kbddevice_project)

pico_sdk_init()

add_executable(kbddevice
    kbddevice.c
    usb_descriptors.c
)

target_include_directories(kbddevice PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(kbddevice PRIVATE
    pico_stdlib
    pico_unique_id
    hardware_gpio
    tinyusb_device
    tinyusb_board
)

pico_add_extra_outputs(kbddevice)

