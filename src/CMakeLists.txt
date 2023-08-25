find_package(OpenSSL REQUIRED)

add_executable(my_server main.cpp)
target_sources(my_server PRIVATE 
  base_connection.cpp
  connection_manager.cpp
  connection.cpp
  mime_types.cpp
  request_handler.cpp
  request_parser.cpp
  request.cpp
  response.cpp
  server.cpp
  ssl_connection.cpp
  string_utils.cpp
)
target_compile_definitions(my_server PRIVATE -DDATA_PATH="${DATA_PATH}")
target_include_directories(my_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(my_server PRIVATE asio::asio spdlog::spdlog OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(my_server PROPERTIES CXX_STANDARD 17)