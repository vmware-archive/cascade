cmake_minimum_required(VERSION 3.10)
project(cascade-test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file (GLOB_RECURSE REGRESSION_DIR "regression/*.cc")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(run_regression harness.cc ${REGRESSION_DIR})
target_link_libraries(run_regression libcascade gtest Threads::Threads ${CMAKE_DL_LIBS})

add_executable(run_benchmark harness.cc benchmark/benchmark.cc)
target_link_libraries(run_benchmark libcascade gtest benchmark Threads::Threads ${CMAKE_DL_LIBS})

add_test(NAME regression COMMAND ${CMAKE_BINARY_DIR}/run_regression WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/..)
