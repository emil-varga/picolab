cmake_minimum_required(VERSION 3.13)
set(PICO_SDK_PATH /home/emil/programming/pico-sdk)
set(PICOTOOL_FORCE_FETCH_FROM_GIT 1)
include(pico_sdk_import.cmake)

project(picolab C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(picolab
    main.c
    scpi_parsing.c
    commands.c
    bmp280.c
)

pico_enable_stdio_usb(picolab 1)
pico_add_extra_outputs(picolab)
target_link_libraries(picolab
    pico_stdlib
    hardware_adc
    hardware_dma
    hardware_i2c)
