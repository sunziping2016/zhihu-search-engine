cmake_minimum_required(VERSION 3.6)

project(zhihu-search-engine)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)


set(SOURCE_FILES query/htmldata.cpp query/mycodecvt.h query/mypair.h query/htmldata.h query/mydir.cpp query/mystack.h query/htmlparser.cpp query/mydir.h query/mystring.h query/htmlparser.h query/myfunctional.h query/myvector.h query/main.cpp query/myhashmap.h query/threadpool.h query/myhashset.h query/trie.cpp query/mycodecvt.cpp query/mylist.h query/trie.h)

add_executable(html_parser ${SOURCE_FILES})

set_target_properties(html_parser PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif(MSVC)

if(UNIX)
	target_link_libraries(html_parser pthread)
endif(UNIX)