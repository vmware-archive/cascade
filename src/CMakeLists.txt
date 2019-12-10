project(cascade C CXX)

include_directories(${FLEX_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

bison_target(verilog_parser cascade/verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/verilog_parser.cc)
flex_target(verilog_lexer cascade/verilog/parse/verilog.ll ${CMAKE_CURRENT_BINARY_DIR}/verilog_lexer.cc)
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
install(DIRECTORY "cascade" DESTINATION ${CMAKE_INSTALL_PREFIX}/src FILES_MATCHING PATTERN "*.h")
