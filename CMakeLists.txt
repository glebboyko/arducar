cmake_minimum_required(VERSION 3.20)
project(arducar)

set(CMAKE_CXX_STANDARD 20)

#including project files
include_directories(.)


#adding executables
add_executable(arducar main.cpp map/env-shot.hpp map/env-shot.cpp)