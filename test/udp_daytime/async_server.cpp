#include <array>
#include <asio.hpp>
#include <ctime>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

using asio::ip::udp;

std::string make_daytime_string() {
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class udp_server {
public:
  udp_server(asio::io_context &io_context)
      : m_socket(io_context, udp::endpoint(udp::v4(), 13)) {
    start_receive();
  }

private:
  void start_receive() {
    m_socket.async_receive_from(
        asio::buffer(m_recv_buffer), m_remote_endpoint,
        [this](const asio::error_code &error, size_t bytes_transferred) {
          handle_receive(error, bytes_transferred);
        });
  }

  void handle_receive(const asio::error_code &error,
                      std::size_t /*bytes_transferred*/) {
    if (!error) {
      auto message = std::make_shared<std::string>(make_daytime_string());

      m_socket.async_send_to(
          asio::buffer(*message), m_remote_endpoint,
          [=](const asio::error_code &error, size_t bytes_transferred) {
            handle_send(message, error, bytes_transferred);
          });

      start_receive();
    } else {
      spdlog::error("{}: {}", error.category().name(), error.message());
    }
  }

  void handle_send(std::shared_ptr<std::string> /*message*/,
                   const asio::error_code &error,
                   std::size_t /*bytes_transferred*/) {
    if (error) {
      spdlog::error("{}: {}", error.category().name(), error.message());
    }
  }

private:
  udp::socket m_socket;
  udp::endpoint m_remote_endpoint;
  std::array<char, 1> m_recv_buffer;
};

int main() {
  try {
    asio::io_context io_context;
    udp_server server(io_context);
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}