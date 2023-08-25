find_package(OpenSSL REQUIRED)

add_executable(ssl_client client.cpp)
target_link_libraries(ssl_client PRIVATE asio::asio spdlog::spdlog OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(ssl_client PROPERTIES CXX_STANDARD 17)

add_executable(ssl_server server.cpp)
target_link_libraries(ssl_server PRIVATE asio::asio spdlog::spdlog OpenSSL::SSL OpenSSL::Crypto)
set_target_properties(ssl_server PROPERTIES CXX_STANDARD 17)

# copy ca.pem and dh4096.pem to ouptut binary
# why there are three pem files ?
file(INSTALL ca.pem dh4096.pem server.pem DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

