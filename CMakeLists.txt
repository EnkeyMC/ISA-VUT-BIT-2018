cmake_minimum_required(VERSION 3.6)
project(isa)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/main.cpp)
add_executable(isa ${SOURCE_FILES})