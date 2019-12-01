cmake_minimum_required(VERSION 3.10)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(cascade cascade.cc)
target_link_libraries(cascade PRIVATE libcascade Threads::Threads)
install(TARGETS cascade RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(cascade_slave cascade_slave.cc)
target_link_libraries(cascade_slave PRIVATE libcascade Threads::Threads)
install(TARGETS cascade_slave RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(de10_config de10_config.c)
target_link_libraries(de10_config)
install(TARGETS de10_config RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(de10_probe de10_probe.cc)
target_link_libraries(de10_probe PRIVATE libcascade Threads::Threads)

add_executable(quartus_server quartus_server quartus_server.cc)
target_link_libraries(quartus_server PRIVATE libcascade Threads::Threads)
install(TARGETS quartus_server RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(sw_fpga sw_fpga.cc)
target_link_libraries(sw_fpga PRIVATE libcascade ncurses Threads::Threads)
install(TARGETS sw_fpga RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
