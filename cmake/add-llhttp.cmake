# code from https://github.com/nodejs/llhttp#using-with-cmake
function(add_llhttp)
  set(options SHARED)
  set(one_value_args VERSION)
  set(multi_value_args "")
  cmake_parse_arguments(ARGS "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
  message(STATUS "add llhttp ${ARGS_VERSION}")
  include(FetchContent)
  FetchContent_Declare(
    llhttp 
    URL https://github.com/nodejs/llhttp/archive/refs/tags/release/${ARGS_VERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
  )
  if(ARGS_SHARED)
    set(BUILD_SHARED_LIBS ON CACHE INTERNAL "")
    set(BUILD_STATIC_LIBS OFF CACHE INTERNAL "")
  else()
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
  endif()
  FetchContent_MakeAvailable(llhttp)
endfunction()
