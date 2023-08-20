#include <array>
#include <asio.hpp>
#include <exception>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>

using asio::ip::tcp;

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      spdlog::error("Usage: sync-client <host>");
      return -1;
    }
    asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
        resolver.resolve(argv[1], "daytime");
    tcp::socket socket(io_context);
    asio::connect(socket, endpoints);
    spdlog::info("connection established, remote endpoint [{}]",
                 socket.remote_endpoint().address().to_string());
    for (;;) {
      std::array<char, 128> buf;
      asio::error_code error;
      size_t len = socket.read_some(asio::buffer(buf), error);
      if (error == asio::error::eof) {
        spdlog::info("connection closed cleanly by peer.");
        break;
      } else if (error) {
        throw asio::system_error(error);
      }
      spdlog::info("receive data: {}, len: {} byte(s)", buf.data(), len);
    }
  } catch (std::exception &e) {
    spdlog::error(e.what());
  }
}