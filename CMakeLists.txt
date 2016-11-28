cmake_minimum_required(VERSION 3.6)
project(html_parser)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra")

set(SOURCE_FILES main.cpp mydir.cpp mycodecvt.cpp htmlparser.cpp htmldata.h htmldata.cpp threadpool.h)
add_executable(html_parser ${SOURCE_FILES})
target_link_libraries(html_parser pthread)
