cmake_minimum_required(VERSION 3.6)
project(html_parser)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES main.cpp mydir.cpp mycodecvt.cpp htmlparser.cpp htmldata.cpp trie.cpp)
add_executable(html_parser ${SOURCE_FILES})

set_target_properties(html_parser PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 14)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif(MSVC)

if(UNIX)
	target_link_libraries(html_parser pthread)
endif(UNIX)