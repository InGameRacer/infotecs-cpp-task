cmake_minimum_required(VERSION 3.10)
project(program2 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Reference dynamic library headers and link it
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../library)

add_executable(program2 main.cpp)

# Link dynamic_lib
target_link_libraries(program2 PRIVATE dynamic_lib)

# Set runtime path (rpath) so the executable can find libdynamic_lib.so at runtime
if(UNIX)
    set_target_properties(program2 PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(program2 PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
    set_target_properties(program2 PROPERTIES LINK_FLAGS "-Wl,-rpath,.:./lib:../lib")
endif()

install(TARGETS program2 DESTINATION ${CMAKE_BINARY_DIR}/bin)
