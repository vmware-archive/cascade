cmake_minimum_required(VERSION 3.10)
project(cascade-test)

file (GLOB_RECURSE REGRESSION_DIR "regression/*.cc")

include_directories(../lib)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(run_regression harness.cc ${REGRESSION_DIR})
target_link_libraries(run_regression libcascade verilog gtest Threads::Threads)

add_executable(run_benchmark harness.cc benchmark/benchmark.cc)
target_link_libraries(run_benchmark libcascade verilog gtest benchmark Threads::Threads)

add_executable(run_microbenchmark harness.cc benchmark/microbenchmark.cc)
target_link_libraries(run_microbenchmark libcascade verilog gtest benchmark Threads::Threads)

add_test(NAME regression 
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_regression 
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
