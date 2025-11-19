cmake_minimum_required(VERSION 3.31)

project(Spire)

set(SPIRE_VERSION "1.5.0")
add_compile_definitions(SPIRE_VERSION="${SPIRE_VERSION}")

set(LIBS_DIRECTORY ${CMAKE_SOURCE_DIR}/Libs)
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(BUILD_MODE_DIR "${LIBS_DIRECTORY}/build-release")
    set(DEBUG_SUFFIX "")
else ()
    set(BUILD_MODE_DIR "${LIBS_DIRECTORY}/build-debug")
    set(DEBUG_SUFFIX "d")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_subdirectory(Spire)
add_subdirectory(SpireVoxel)
add_subdirectory(Game)