cmake_minimum_required(VERSION 3.20)
project(arducar_raspberry)

set(CMAKE_CXX_STANDARD 20)

#setting up wxWidgets
if (APPLE)
    set(wxWidgets_ROOT_DIR </opt/homebrew/bin/wx-config>)
else ()
    set(wxWidgets_ROOT_DIR </usr/include/wx-3.0>)
endif ()
set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

#including project files
include_directories(libs/c_tcp_lib/cpp/include)
include_directories(libs/clogger_lib/include)
include_directories(libs/primitives-to-image-translator/include)

include_directories(map)
include_directories(frontend/frame)

#adding executables
add_executable(${PROJECT_NAME} main.cpp
        frontend/frame/frame.cpp
        map/env-shot.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/libs/built/lib_ptit.a)
target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES})