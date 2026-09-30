cmake_minimum_required(VERSION 3.15)
project(arptool CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8 /W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_executable(arptool
    main.cpp
    list_arp.cpp
    add_arp.cpp
    delete_arp.cpp
    utils.cpp
)

target_link_libraries(arptool PRIVATE ws2_32 iphlpapi shell32)
