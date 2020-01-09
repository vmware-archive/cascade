project(cascade C CXX)

configure_file(${CMAKE_SOURCE_DIR}/src/cascade/common/cmake.in ${CMAKE_CURRENT_BINARY_DIR}/../codegen/cmake.h)

if(${BISON_VERSION} VERSION_GREATER 3.2.9) 
  bison_target(verilog_parser cascade/verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/../codegen/verilog_parser.cc COMPILE_FLAGS --warnings=none)
else(${BISON_VERSION} VERSION_GREATER 3.2.9)
  bison_target(verilog_parser cascade/verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/../codegen/verilog_parser.cc)
endif(${BISON_VERSION} VERSION_GREATER 3.2.9)
flex_target(verilog_lexer cascade/verilog/parse/verilog.ll ${CMAKE_CURRENT_BINARY_DIR}/../codegen/verilog_lexer.cc)
add_flex_bison_dependency(verilog_lexer verilog_parser)

file (GLOB_RECURSE SRC "*.cc")
add_library(libcascade SHARED STATIC 
  ${BISON_verilog_parser_OUTPUTS} 
  ${FLEX_verilog_lexer_OUTPUTS}
  ${SRC}
)
set_target_properties(libcascade PROPERTIES PREFIX "")

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
  target_compile_options(libcascade PRIVATE -Wno-psabi)
endif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")

install(TARGETS libcascade DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY cascade DESTINATION ${CMAKE_INSTALL_PREFIX}/include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY cascade/codegen DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/codegen FILES_MATCHING PATTERN "*.h")
