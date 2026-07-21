cmake_minimum_required(VERSION 3.10)
project(program1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pthread library
find_package(Threads REQUIRED)

# Reference dynamic library headers and link it
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../library)

add_executable(program1 main.cpp)

# Link dynamic_lib and pthread
target_link_libraries(program1 PRIVATE dynamic_lib Threads::Threads)

# Set runtime path (rpath) so the executable can find libdynamic_lib.so at runtime
if(UNIX)
    set_target_properties(program1 PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(program1 PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
    set_target_properties(program1 PROPERTIES LINK_FLAGS "-Wl,-rpath,.:./lib:../lib")
endif()

install(TARGETS program1 DESTINATION ${CMAKE_BINARY_DIR}/bin)
