cmake_minimum_required(VERSION 3.10)

add_executable(cascade cascade.cc)
target_link_libraries(cascade PRIVATE libcascade mongoose)

add_executable(de10_probe de10_probe.cc)
target_link_libraries(de10_probe PRIVATE libcascade mongoose)

add_executable(quartus_server quartus_server quartus_server.cc)
target_link_libraries(quartus_server PRIVATE libcascade mongoose)

add_executable(swfpga sw_fpga.cc)
target_link_libraries(swfpga PRIVATE libcascade mongoose CONAN_PKG::ncurses)
