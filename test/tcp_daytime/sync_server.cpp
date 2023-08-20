#include <asio.hpp>
#include <chrono>
#include <spdlog/fmt/chrono.h>
#include <spdlog/spdlog.h>
#include <string>

using asio::ip::tcp;

// implement a simple daytime protocol
// https://www.ietf.org/rfc/rfc867.txt
std::string make_daytime_string() {
  std::time_t now = std::time(nullptr);
  return ctime(&now);
}

int main(int argc, const char *argv[]) {
  try {
    asio::io_context io_context;
    // creater tcp server, listen to port 13, IPv4
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 13));
    spdlog::info("start synchronous TCP daytime server at 0.0.0.0:{}", 13);
    for (;;) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);
      spdlog::info("connection established, remote endpoint [{}]",
                   socket.remote_endpoint().address().to_string());
      auto message = make_daytime_string();
      spdlog::info("send: {}, len: {} byte(s)", message, message.size());
      asio::error_code ignored_error;
      asio::write(socket, asio::buffer(message), ignored_error);
    }
  } catch (std::exception &e) {
    spdlog::error("exception: {}", e.what());
  }
}