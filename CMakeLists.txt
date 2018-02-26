cmake_minimum_required(VERSION 2.8)
project(nmines)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(nmines main.c)
target_link_libraries(nmines ${CURSES_LIBRARIES})