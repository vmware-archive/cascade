project(cascade C CXX)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCE_FILES
	cascade.h
	cascade.cc
)

file (GLOB_RECURSE SOURCE_DIRECTORY "../*.cc")

add_library(libcascade SHARED STATIC ${SOURCE_FILES} ${SOURCE_DIRECTORY})
set_target_properties(libcascade PROPERTIES PREFIX "")

target_link_libraries(libcascade verilog)

install(TARGETS libcascade DESTINATION ${CASCADE_INSTALL_LIB_DIR})
install(FILES cascade.h DESTINATION ${CASCADE_INSTALL_INCLUDE_DIR})
