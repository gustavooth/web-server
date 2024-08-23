cmake_minimum_required(VERSION 3.29)

project(server C)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 23)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_C_COMPILER clang)

file(GLOB_RECURSE SOURCE "src/*.c")

include_directories(
    ${CMAKE_SOURCE_DIR}/src/
    /usr/include
)

add_executable(server ${SOURCE})