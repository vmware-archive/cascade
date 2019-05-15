cmake_minimum_required(VERSION 3.10)
project(cascade-test)

file (GLOB_RECURSE REGRESSION_DIR "regression/*.cc")
file (GLOB_RECURSE BENCHMARK_DIR "benchmark/*.cc")

include_directories(../lib)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(regression harness.cc ${REGRESSION_DIR})
target_link_libraries(regression CONAN_PKG::gtest)
target_link_libraries(regression libcascade verilog)

add_executable(benchmark harness.cc ${BENCHMARK_DIR})
target_link_libraries(benchmark CONAN_PKG::gtest)
target_link_libraries(benchmark libcascade verilog)

add_test(NAME regression 
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/regression 
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})