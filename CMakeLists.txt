cmake_minimum_required(VERSION 2.8.4)
project(Actuator)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/tool.cc
    src/tool.h
    src/interp.cc
    src/interp.h
    src/lexer.cc
    src/lexer.h
    src/main.cc
    src/parser.cc
    src/parser.h
    LICENSE.md
    README.md)

add_definitions(-Wall)
add_executable(Actuator ${SOURCE_FILES})