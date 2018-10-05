cmake_minimum_required(VERSION 3.5)
project(isa)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ")

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

set(SOURCE_FILES src/main.cpp src/exceptions.cpp src/exceptions.h src/SSLWrapper.cpp src/SSLWrapper.h src/Http.cpp src/Http.h src/Url.cpp src/Url.h)
add_executable(isa ${SOURCE_FILES})
target_include_directories(isa PRIVATE /usr/include/libxml2)
target_link_libraries(isa ${OPENSSL_LIBRARIES})