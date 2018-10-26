cmake_minimum_required(VERSION 3.5)
project(isa)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}")

SET(CMAKE_BUILD_TYPE Debug)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR} "lib")

set(SOURCE_FILES src/main.cpp src/exceptions.cpp src/exceptions.h src/SSLWrapper.cpp src/SSLWrapper.h src/Http.cpp src/Http.h src/Url.cpp src/Url.h src/debug.h src/utils.cpp src/utils.h src/Feed.cpp src/Feed.h)
add_executable(feedreader ${SOURCE_FILES} ${PUGIXML_FILES})
target_include_directories(feedreader PRIVATE /usr/include/libxml2)
target_link_libraries(feedreader ${OPENSSL_LIBRARIES})