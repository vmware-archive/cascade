project(cascade C CXX)

include_directories(${FLEX_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

bison_target(verilog_parser ../verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/verilog_parser.cc)
flex_target(verilog_lexer ../verilog/parse/verilog.ll ${CMAKE_CURRENT_BINARY_DIR}/verilog_lexer.cc)
add_flex_bison_dependency(verilog_lexer verilog_parser)

file (GLOB SOURCE_FILES
	cascade.h
	cascade.cc
)
file (GLOB_RECURSE SOURCE_DIRECTORY "../*.cc")

add_library(libcascade SHARED STATIC 
  ${BISON_verilog_parser_OUTPUTS} 
  ${FLEX_verilog_lexer_OUTPUTS}
  ${SOURCE_FILES} 
  ${SOURCE_DIRECTORY}
)
set_target_properties(libcascade PROPERTIES PREFIX "")

install(TARGETS libcascade DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cascade)
install(FILES cascade.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/cascade)
install(FILES cascade_slave.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/cascade)
