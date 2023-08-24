add_executable(test_spdlog test_spdlog.cpp)
target_link_libraries(test_spdlog spdlog::spdlog)
set_property(TARGET test_spdlog PROPERTY CXX_STANDARD 17)

add_executable(test_asio test_asio.cpp)
target_link_libraries(test_asio spdlog::spdlog asio::asio)
set_property(TARGET test_asio PROPERTY CXX_STANDARD 17)

add_executable(test_asio_mt test_asio_mt.cpp)
target_link_libraries(test_asio_mt spdlog::spdlog asio::asio)
set_property(TARGET test_asio_mt PROPERTY CXX_STANDARD 17)

add_executable(test_udp_checksum test_udp_checksum.cpp)
target_link_libraries(test_udp_checksum spdlog::spdlog asio::asio)
set_property(TARGET test_udp_checksum PROPERTY CXX_STANDARD 17)

add_subdirectory(tcp_daytime)
add_subdirectory(udp_daytime)
add_subdirectory(ssl)
add_subdirectory(http/server)

add_executable(test_request_parser test_request_parser.cpp)
target_sources(test_request_parser PRIVATE http/server/request_parser.cpp)
target_include_directories(test_request_parser PRIVATE http/server)
target_link_libraries(test_request_parser spdlog::spdlog)
target_compile_definitions(test_request_parser PRIVATE -DDATA_PATH="${DATA_PATH}")
set_property(TARGET test_request_parser PROPERTY CXX_STANDARD 17)
