cmake_minimum_required(VERSION 3.20)
project(arducar_raspberry)

set(CMAKE_CXX_STANDARD 20)

#including project files
include_directories(libs/c_tcp_lib/cpp/include)
include_directories(libs/clogger_lib/include)
include_directories(libs/primitives-to-image-translator/include)

include_directories(map)

#adding executables
add_executable(arducar main.cpp
        map/env-shot.cpp)