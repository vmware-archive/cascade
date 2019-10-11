cmake_minimum_required(VERSION 3.2)
project(cascade)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FLEX 2.6.0)
find_package(BISON)

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

set(CASCADE_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CASCADE_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(CASCADE_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

include_directories(..)
include_directories(src)
include_directories(${FLEX_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Verilog parser/lexer
bison_target(verilog_parser src/verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/verilog_parser.cc)
flex_target(verilog_lexer src/verilog/parse/verilog.ll ${CMAKE_CURRENT_BINARY_DIR}/verilog_lexer.cc)
add_flex_bison_dependency(verilog_lexer verilog_parser)
add_library(verilog ${BISON_verilog_parser_OUTPUTS} ${FLEX_verilog_lexer_OUTPUTS})

add_subdirectory(src/cascade)
add_subdirectory(tools)
add_subdirectory(test)
