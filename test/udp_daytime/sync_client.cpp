#include <array>
#include <asio.hpp>
#include <iostream>

using asio::ip::udp;

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::endpoint receiver_endpoint =
        *resolver.resolve(udp::v4(), argv[1], "daytime").begin();

    udp::socket socket(io_context);
    socket.open(udp::v4());

    // 由于 UDP
    // 是无连接点，我们先向服务器发一个空消息，表示我们需要请求服务，然后接受服务器返回的消息即可
    std::array<char, 1> send_buf = {{0}};
    socket.send_to(asio::buffer(send_buf), receiver_endpoint);

    std::array<char, 128> recv_buf;
    udp::endpoint sender_endpoint;
    size_t len = socket.receive_from(asio::buffer(recv_buf), sender_endpoint);

    std::cout.write(recv_buf.data(), len);
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}