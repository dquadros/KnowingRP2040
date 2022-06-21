cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(usbserial_project)

pico_sdk_init()

add_executable(usbserial
    usbserial.c
    usb_descriptors.c
)

target_include_directories(usbserial PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(usbserial PRIVATE
    pico_stdlib
    pico_unique_id
    hardware_uart
    tinyusb_device
    tinyusb_board
)

pico_add_extra_outputs(usbserial)

