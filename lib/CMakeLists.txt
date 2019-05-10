project(cascade C CXX)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCE_FILES
	cascade.h
	cascade.cc
)

file (GLOB_RECURSE SOURCE_DIRECTORY "../src/*.cc")

bison_target(verilog_parser ../src/verilog/parse/verilog.yy ${CMAKE_CURRENT_BINARY_DIR}/verilog_parser.cc)
flex_target(verilog_lexer ../src/verilog/parse/verilog.ll ${CMAKE_CURRENT_BINARY_DIR}/verilog_lexer.cc)
add_flex_bison_dependency(verilog_lexer verilog_parser)

add_library(libcascade SHARED STATIC ${SOURCE_FILES} ${SOURCE_DIRECTORY} ${BISON_verilog_parser_OUTPUTS} ${FLEX_verilog_lexer_OUTPUTS})

# build mongoose (TODO: remove from source)
add_library(mongoose SHARED STATIC ../ext/mongoose/mongoose.c)

target_link_libraries(libcascade mongoose)

install(TARGETS libcascade DESTINATION ${CASCADE_INSTALL_LIB_DIR})
install(FILES cascade.h DESTINATION ${CASCADE_INSTALL_INCLUDE_DIR})