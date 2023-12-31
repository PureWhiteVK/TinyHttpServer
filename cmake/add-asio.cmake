function(add_asio)
  set(options "")
  set(one_value_args ASIO_DIR)
  set(multi_value_args "")
  cmake_parse_arguments(ARGS "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})
  message(STATUS "add asio [header only]")
  add_library(asio INTERFACE)
  target_include_directories(asio INTERFACE ${ARGS_ASIO_DIR}/asio/include)
  add_library(asio::asio ALIAS asio)
endfunction()
