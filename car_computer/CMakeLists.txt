cmake_minimum_required(VERSION 3.20)
project(.)

set(CMAKE_CXX_STANDARD 20)

#including project files
include_directories(../libs)
include_directories(../libs/c_tcp_lib/cpp/include)
include_directories(../libs/clogger_lib/include)
include_directories(../libs/ptit_lib/include)

include_directories(../map/include)

#adding executables
add_executable(${PROJECT_NAME}
        ../map/source/bitmap.cpp
        ../map/source/env-shot.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/libs/built/lib_ptit.a)