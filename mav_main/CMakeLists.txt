cmake_minimum_required(VERSION 3.22)
project(LIGHTNING_GPU)

include(FetchContent)
include(ccache.cmake)
include(git_info.cmake)

option(common_warnings "Use file warnings.cmake " ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
# set(PARTY3 ../3rdparty)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD          "20")
set(CMAKE_CXX_FLAGS_RELEASE     "-O2")
set(CMAKE_CXX_FLAGS_DEBUG       "-O0 -g")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_library(LIBRT rt)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rk_ipc )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gpu_ipc )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app )
add_subdirectory(3rd_party/unp_cp)

install(FILES app/gpuConfig.xml
        DESTINATION cfg
        COMPONENT gpu_cfg
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
        )

install(TARGETS lightning_gpu
        DESTINATION bin
        RUNTIME
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                    GROUP_EXECUTE GROUP_READ
                    WORLD_EXECUTE WORLD_READ
        )

include_directories(
    ${PARTY3}/mavlink/c_library_v1/common
)
