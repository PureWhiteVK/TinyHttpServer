add_executable(sync-client sync_client.cpp)
target_link_libraries(sync-client spdlog::spdlog asio::asio)
set_property(TARGET sync-client PROPERTY CXX_STANDARD 17)

add_executable(sync-server sync_server.cpp)
target_link_libraries(sync-server spdlog::spdlog asio::asio)
set_property(TARGET sync-server PROPERTY CXX_STANDARD 17)

add_executable(async-server async_server.cpp)
target_link_libraries(async-server spdlog::spdlog asio::asio)
set_property(TARGET async-server PROPERTY CXX_STANDARD 17)