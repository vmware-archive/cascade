cmake_minimum_required(VERSION 3.10)
project(cascade-test)

find_package(gtest)

file (GLOB_RECURSE REGRESSION_DIR "regression/*.cc")
file (GLOB_RECURSE BENCHMARK_DIR "benchmark/*.cc")

include_directories(../lib)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


bison_target(verilog_parser ../src/verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/verilog_parser.cc)
flex_target(verilog_lexer ../src/verilog/parse/verilog.ll ${CMAKE_CURRENT_BINARY_DIR}/verilog_lexer.cc)
add_flex_bison_dependency(verilog_lexer verilog_parser)

add_executable(regression harness.cc ${REGRESSION_DIR} ${BISON_verilog_parser_OUTPUTS} ${FLEX_verilog_lexer_OUTPUTS})
target_link_libraries(regression CONAN_PKG::gtest)
target_link_libraries(regression libcascade)

add_executable(benchmark harness.cc ${BENCHMARK_DIR} ${BISON_verilog_parser_OUTPUTS} ${FLEX_verilog_lexer_OUTPUTS})
target_link_libraries(benchmark CONAN_PKG::gtest)
target_link_libraries(benchmark libcascade)

add_test(NAME regression 
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/regression 
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME benchmark
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/benchmark
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})