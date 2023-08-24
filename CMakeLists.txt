cmake_minimum_required(VERSION 3.20.0)
project(TinyHttpServer)

add_subdirectory(cmake)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/utf-8")
endif()

if(WIN32)
    get_windows_version(OUTPUT_NAME win_ver)
    message(STATUS "windows version: ${win_ver}")
    add_definitions(-D_WIN32_WINNT=${win_ver} -DWINVER=${win_ver})
endif()

cmake_path(SET DATA_PATH NORMALIZE ${PROJECT_SOURCE_DIR}/data)

add_subdirectory(extern)
add_subdirectory(src)
add_subdirectory(test)