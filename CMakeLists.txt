cmake_minimum_required(VERSION 3.13)
project(etwolf_server_demo C)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION gnu)
set(CMAKE_SYSTEM_PROCESSOR i386)
set(CMAKE_C_FLAGS -m32)

set(SUBHOOK_STATIC ON)
set(SUBHOOK_FORCE_32BIT ON)

add_subdirectory(subhook)
add_subdirectory(zlib)

add_library(etwolf_server_demo SHARED
    src/hook.h
    src/hook.c
    src/etwolf.h
    src/record.h
    src/record.c
)

target_link_libraries(etwolf_server_demo -static subhook -static zlibstatic)