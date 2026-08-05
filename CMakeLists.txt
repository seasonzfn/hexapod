cmake_minimum_required(VERSION 3.20)
project(hexapod LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(hexapod
    src/Hexapod.cpp
    src/leg.cpp
    main.cpp
)

target_include_directories(hexapod PRIVATE include)

target_compile_features(hexapod PRIVATE "cxx_std_${CMAKE_CXX_STANDARD}")

target_compile_options(hexapod PRIVATE -Wall -Wextra -Wshadow -Wnull-dereference -pedantic)


target_compile_options(hexapod PRIVATE
    $<$<CONFIG:Debug>:-Og -ggdb3>
    $<$<CONFIG:Release>:-O2>
)