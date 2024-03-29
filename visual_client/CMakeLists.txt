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

include_directories(../share)

include_directories(include)

#adding executables
add_executable(${PROJECT_NAME} main.cpp
        source/frame.cpp
        source/data_processor.cpp
        source/communicator.cpp

        ../share/map/source/bitmap.cpp
        ../share/map/source/env-shot.cpp
        ../share/map/source/map_analyzer.cpp

        ../share/messages/messages.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../libs/built/lib_ptit.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../libs/built/lib_c_tcp.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})