#include <asio.hpp>
#include <chrono>
#include <memory>
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

// TODO: what does std::enable_shared_from_this do ?
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
  using pointer = std::shared_ptr<TcpConnection>;

  pointer getptr() { return shared_from_this(); }

  [[nodiscard]] static pointer create(asio::io_context &io_context) {
    spdlog::info("create tcp connection!");
    return pointer(new TcpConnection(io_context));
  }

  tcp::socket &socket() { return m_socket; }

  void start() {
    spdlog::info("start new tcp connection!");
    m_message = make_daytime_string();
    spdlog::info("send {} to {}", m_message,
                 m_socket.remote_endpoint().address().to_string());
    asio::async_write(m_socket, asio::buffer(m_message),
                      [connection = getptr()](const asio::error_code &error,
                                              size_t bytes_transferred) {
                        connection->handle_write(error, bytes_transferred);
                      });
  }

  ~TcpConnection() { spdlog::info("close tcp connection!"); }

private:
  TcpConnection(asio::io_context &io_context) : m_socket(io_context) {}

  void handle_write(const asio::error_code &error, size_t bytes_transferred) {
    spdlog::info("write {} byte(s)", bytes_transferred);
  }

private:
  tcp::socket m_socket;
  std::string m_message;
};

class TcpServer {
public:
  TcpServer(asio::io_context &io_context)
      : m_io_context(io_context),
        m_acceptor(io_context, tcp::endpoint(tcp::v4(), 13)) {
    start_accept();
  }

private:
  void start_accept() {
    spdlog::info("start accept!");
    // 此处我们必须使用智能指针，否则该函数执行完 new_connection 就会被析构掉了
    TcpConnection::pointer new_connection = TcpConnection::create(m_io_context);
    m_acceptor.async_accept(new_connection->socket(),
                            [this, connection = new_connection->getptr()](
                                const asio::error_code &error) {
                              handle_accept(connection, error);
                            });
  }

  void handle_accept(TcpConnection::pointer new_connection,
                     const asio::error_code &error) {
    spdlog::info(
        "connection established, remote endpoint: {}.{}",
        new_connection->socket().remote_endpoint().address().to_string(),
        new_connection->socket().remote_endpoint().port());
    if (!error) {
      new_connection->start();
    } else {
      spdlog::error("{}: {}", error.category().name(), error.message());
    }
    start_accept();
  }

private:
  asio::io_context &m_io_context;
  tcp::acceptor m_acceptor;
};

int main(int argc, const char *argv[]) {
  try {
    asio::io_context io_context;
    TcpServer tcp_server(io_context);
    io_context.run();
  } catch (std::exception &e) {
    spdlog::error("exception: {}", e.what());
  }
}