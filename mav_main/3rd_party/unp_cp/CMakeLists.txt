
cmake_minimum_required(VERSION 3.16)

project(unp_cp)

set(CMAKE_CXX_STANDARD          "20")
set(CMAKE_CXX_FLAGS_RELEASE     "-O2")
set(CMAKE_CXX_FLAGS_DEBUG       "-O0 -g")

include_directories(inc)
include_directories(src)
include_directories(src/unp_rpc)

file(GLOB CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cc )
file(GLOB HPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.h)
file(GLOB HPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h ${CMAKE_CURRENT_SOURCE_DIR}/inc/**/*.h)

add_library(${PROJECT_NAME} STATIC ${CPP_SOURCES} ${HPP_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )