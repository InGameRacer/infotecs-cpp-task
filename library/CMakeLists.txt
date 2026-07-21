cmake_minimum_required(VERSION 3.10)
project(dynamic_lib CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define building macro for library
add_compile_definitions(BUILDING_DYNAMIC_LIB)

# Create shared library
add_library(dynamic_lib SHARED dynamic_lib.cpp dynamic_lib.h)

# Install destination
install(TARGETS dynamic_lib DESTINATION ${CMAKE_BINARY_DIR}/lib)
