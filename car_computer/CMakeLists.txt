cmake_minimum_required(VERSION 3.20)
project(arducar)

set(CMAKE_CXX_STANDARD 20)

#including project files
include_directories(../libs)
include_directories(../libs/c_tcp_lib/cpp/include)
include_directories(../libs/clogger_lib/include)
include_directories(../libs/ptit_lib/include)

include_directories(include)

include_directories(../share)
include_directories(../map)

#adding executables
add_executable(${PROJECT_NAME}
        main.cpp
        source/runners.cpp
        source/control_center.cpp

        source/communicators.cpp
        ../share/messages/messages.cpp

        ../share/map/source/env-shot.cpp
        ../share/map/source/map_analyzer.cpp
        ../share/map/source/bitmap.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../libs/built/lib_ptit.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../libs/built/lib_c_tcp.a)