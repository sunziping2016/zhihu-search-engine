cmake_minimum_required(VERSION 3.6)

project(zhihu-search-engine)

set(CMAKE_CXX_STANDARD 14)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif(MSVC)

set(SOURCE_FILES query/htmldata.cpp query/mycodecvt.h query/mypair.h query/htmldata.h query/mydir.cpp query/mystack.h query/htmlparser.cpp query/mydir.h query/mystring.h query/htmlparser.h query/myfunctional.h query/myvector.h query/main.cpp query/myhashmap.h query/threadpool.h query/myhashset.h query/trie.cpp query/mycodecvt.cpp query/mylist.h query/trie.h)

add_executable(query ${SOURCE_FILES})
set_target_properties(query PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(UNIX)
	target_link_libraries(query pthread)
endif(UNIX)

set(DLL_SOURCE_FILES query/htmldata.cpp query/mycodecvt.h query/mypair.h query/htmldata.h query/mydir.cpp query/mystack.h query/htmlparser.cpp query/mydir.h query/mystring.h query/htmlparser.h query/myfunctional.h query/myvector.h query/export.cpp query/myhashmap.h query/threadpool.h query/myhashset.h query/trie.cpp query/mycodecvt.cpp query/mylist.h query/trie.h)
add_library(libquery SHARED ${DLL_SOURCE_FILES})

set_target_properties(libquery PROPERTIES
        OUTPUT_NAME "query"
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gui)