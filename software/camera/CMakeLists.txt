cmake_minimum_required (VERSION 3.0)
project (visor)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package (V4L2 REQUIRED)
find_package (FMT REQUIRED)
find_package (SDL2 REQUIRED)

include_directories (
        ${V4L2_INCLUDE_DIR}
        ${FMT_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIR})

add_executable (
        visor
        main.cpp
        device.cpp device.h
        buffers.cpp buffers.h
        result.h result_message.h
        hex_formatter.cpp hex_formatter.h)

target_link_libraries (
        visor
        ${V4L2_LIBRARIES}
        ${SDL2_LIBRARY}
        fmt::fmt)
