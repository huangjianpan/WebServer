cmake_minimum_required(VERSION 3.0.2)

project(WebServer)

include_directories(./include)

set(CXX_FLAGS
        -Wall
        -std=c++11
        -lpthread
        -Wno-unused-parameter
        -O3
        )

link_libraries(pthread)

set(CMAKE_CXX_COMPILER "g++")

aux_source_directory(./src SRC)

add_executable(main ${SRC} main.cpp)