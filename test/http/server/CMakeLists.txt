add_executable(http_server main.cpp)
target_sources(http_server PRIVATE 
  connection_manager.cpp
  connection.cpp
  mime_types.cpp
  reply.cpp
  request_handler.cpp
  request_parser.cpp
  server.cpp
)
target_include_directories(http_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(http_server PRIVATE asio::asio spdlog::spdlog)
set_target_properties(http_server PROPERTIES CXX_STANDARD 17)