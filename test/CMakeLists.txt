cmake_minimum_required(VERSION 3.10)
project(cascade-test)

file (GLOB_RECURSE REGRESSION_DIR "regression/*.cc")

include_directories(../lib)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(regression harness.cc ${REGRESSION_DIR})
target_link_libraries(regression CONAN_PKG::gtest)
target_link_libraries(regression libcascade uuid verilog)

add_executable(benchmark harness.cc benchmark/benchmark.cc)
target_link_libraries(benchmark CONAN_PKG::gtest CONAN_PKG::google-benchmark)
target_link_libraries(benchmark libcascade uuid verilog)

add_executable(microbenchmark harness.cc benchmark/microbenchmark.cc)
target_link_libraries(microbenchmark CONAN_PKG::gtest CONAN_PKG::google-benchmark)
target_link_libraries(microbenchmark libcascade uuid verilog)

add_test(NAME regression 
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/regression 
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
