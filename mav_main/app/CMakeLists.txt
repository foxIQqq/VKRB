cmake_minimum_required(VERSION 3.22)
project(lightning_gpu)

file(GLOB CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB HPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

set(PARTY3 ../3rdparty)

find_package(LibXml2 REQUIRED)

include_directories(
    ${PARTY3}/mavlink/c_library_v1/common
)

add_executable(${PROJECT_NAME} ${CPP_SOURCES} ${HPP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LIBXML2_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ${LIBRT}
    ${LIBXML2_LIBRARIES}
    rk_ipc
    gpu_ipc
    unp_cp
    )

target_compile_options(lightning_gpu PRIVATE
  -Wno-address-of-packed-member
)
