cmake_minimum_required(VERSION 3.20)
project(hexapod LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(hexapod
    src/Hexapod.cpp
    src/leg.cpp
    src/ServoDriver.cpp
    main.cpp
)

target_include_directories(hexapod PRIVATE include)

target_compile_features(hexapod PRIVATE "cxx_std_${CMAKE_CXX_STANDARD}")

if(MSVC)
    target_compile_options(hexapod PRIVATE /W4)
else()
    target_compile_options(hexapod PRIVATE -Wall -Wextra -Wshadow -Wnull-dereference -pedantic)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_compile_options(hexapod PRIVATE /Od /Zi)
    else()
        target_compile_options(hexapod PRIVATE -Og -ggdb3)
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(NOT MSVC)
        target_compile_options(hexapod PRIVATE -O2)
    endif()
endif()