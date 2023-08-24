add_executable(my_server main.cpp)
target_sources(my_server PRIVATE 
  connection_manager.cpp
  connection.cpp
  mime_types.cpp
  response.cpp
  request_handler.cpp
  request_parser.cpp
  server.cpp
)
target_include_directories(my_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(my_server PRIVATE asio::asio spdlog::spdlog)
set_target_properties(my_server PROPERTIES CXX_STANDARD 17)