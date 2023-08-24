# code modified from https://github.com/antlr/antlr4/blob/dev/runtime/Cpp/demo/CMakeLists.txt
cmake_minimum_required(VERSION 3.20.0)
project(antlr4-cpp-demo)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# compiler must be 17
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif()

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE $ENV{ANTLR_EXECUTABLE})
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

find_package(fmt REQUIRED)

include(CMakePrintHelpers)
cmake_print_variables(ANTLR_VERSION)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# Specify the version of the antlr4 library needed for this project.
# By default the latest version of antlr4 will be used.  You can specify a
# specific, stable version by setting a repository tag value or a link
# to a zip file containing the libary source.
# set(ANTLR4_TAG 4.13.0)
set(ANTLR4_ZIP_REPOSITORY https://github.com/antlr/antlr4/archive/refs/tags/${ANTLR_VERSION}.zip)
cmake_print_variables(ANTLR4_ZIP_REPOSITORY)

# add external build for antlrcpp
include(ExternalAntlr4Cpp)

add_executable(demo main.cpp)

antlr_target(CsvParser CSV.g4 LEXER PARSER PACKAGE csv_parser)

target_sources(demo PRIVATE ${ANTLR_CsvParser_CXX_OUTPUTS})
target_compile_definitions(demo PRIVATE -DANTLR4CPP_STATIC)
target_include_directories(demo PRIVATE ${ANTLR_CsvParser_OUTPUT_DIR})

target_link_libraries(demo PRIVATE antlr4_static fmt::fmt)