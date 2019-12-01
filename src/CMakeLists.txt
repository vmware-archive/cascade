project(cascade C CXX)

include_directories(${FLEX_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

bison_target(verilog_parser verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/verilog_parser.cc)
flex_target(verilog_lexer verilog/parse/verilog.ll ${CMAKE_CURRENT_BINARY_DIR}/verilog_lexer.cc)
add_flex_bison_dependency(verilog_lexer verilog_parser)

file (GLOB_RECURSE SRC "*.cc")
add_library(libcascade SHARED STATIC 
  ${BISON_verilog_parser_OUTPUTS} 
  ${FLEX_verilog_lexer_OUTPUTS}
  ${SRC}
)
set_target_properties(libcascade PROPERTIES PREFIX "")
install(TARGETS libcascade DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(DIRECTORY "common" DESTINATION ${CMAKE_INSTALL_PREFIX}/src/cascade FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "runtime" DESTINATION ${CMAKE_INSTALL_PREFIX}/src/cascade FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "target" DESTINATION ${CMAKE_INSTALL_PREFIX}/src/cascade FILES_MATCHING PATTERN "*.h")
install(DIRECTORY "verilog" DESTINATION ${CMAKE_INSTALL_PREFIX}/src/cascade FILES_MATCHING PATTERN "*.h")
