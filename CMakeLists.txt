cmake_minimum_required(VERSION 3.2)
project(cascade)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLEX 2.6.0 REQUIRED)
find_package(BISON 3.2.0 REQUIRED)

if (APPLE)
    set(cxxlib libc++)
    set(force_compiler "compiler=apple-clang")
else ()
    set(cxxlib libstdc++11)
    set(force_compiler "")
endif()

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^armv7*")
    set(force_arch "armv7")
else()
    set(force_arch ${CMAKE_SYSTEM_PROCESSOR})
endif()

include(CTest)
enable_testing()

if (COVERAGE STREQUAL "TRUE")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/.travis/cmake)
include(CodeCoverage)
setup_target_for_coverage_lcov(NAME cascade_coverage 
EXECUTABLE ctest -R regression --verbose
DEPENDENCIES regression)

set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif()

include_directories(.)
include_directories(src/cascade)
include_directories(share/cascade)

add_subdirectory(include)
add_subdirectory(share)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)
