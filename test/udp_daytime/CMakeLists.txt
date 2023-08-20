add_executable(udp-sync-client sync_client.cpp)
target_link_libraries(udp-sync-client spdlog::spdlog asio::asio)
set_property(TARGET udp-sync-client PROPERTY CXX_STANDARD 17)

add_executable(udp-sync-server sync_server.cpp)
target_link_libraries(udp-sync-server spdlog::spdlog asio::asio)
set_property(TARGET udp-sync-server PROPERTY CXX_STANDARD 17)

add_executable(udp-async-server async_server.cpp)
target_link_libraries(udp-async-server spdlog::spdlog asio::asio)
set_property(TARGET udp-async-server PROPERTY CXX_STANDARD 17)