cmake_minimum_required(VERSION 3.30)
project(ucpm)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(3rd)

add_executable(ucpm
    src/main.c
)
target_link_libraries(ucpm PRIVATE Z80)
