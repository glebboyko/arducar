cmake_minimum_required(VERSION 3.20)
project(arducar_visual_client)

set(CMAKE_CXX_STANDARD 20)

#setting up wxWidgets
set(wxWidgets_ROOT_DIR </opt/homebrew/bin/wx-config>)
set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

#including project files
include_directories(../libs)
include_directories(../libs/c_tcp_lib/cpp/include)
include_directories(../libs/clogger_lib/include)
include_directories(../libs/ptit_lib/include)

include_directories(../map/include)
include_directories(../connection_support)
include_directories(frame)

#adding executables
add_executable(${PROJECT_NAME} main.cpp
        frame/frame.cpp
        ../map/source/bitmap.cpp
        ../map/source/env-shot.cpp
        ../connection_support/support.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../libs/built/lib_ptit.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../libs/built/lib_c_tcp.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})