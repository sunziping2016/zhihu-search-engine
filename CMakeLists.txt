cmake_minimum_required(VERSION 3.6)
project(html_parser)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp mydir.cpp mycodecvt.cpp)
add_executable(html_parser ${SOURCE_FILES})