
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pca_hexapod C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pca_hexapod
    main.cpp
    src/pca9685.cpp
    src/servo.cpp
)

target_include_directories(pca_hexapod PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(pca_hexapod
    pico_stdlib
    hardware_i2c
    hardware_flash
    pico_flash
)

pico_enable_stdio_usb(pca_hexapod 1)
pico_enable_stdio_uart(pca_hexapod 0)

pico_add_extra_outputs(pca_hexapod)
