#include "server.hpp"
#include <signal.h>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

namespace http {
namespace server {

server::server(const std::string &address, const std::string &port,
               const std::string &doc_root)
    : m_context(1), m_signal_set(m_context), m_acceptor(m_context) {
  m_connection_manager = std::make_shared<connection_manager>();
  m_request_handler = std::make_shared<request_handler>(doc_root);
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  m_signal_set.add(SIGINT);
  m_signal_set.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

  do_await_stop();

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  asio::ip::tcp::resolver resolver(m_context);
  asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
  m_acceptor.open(endpoint.protocol());
  m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  m_acceptor.bind(endpoint);
  m_acceptor.listen();
  std::string address_str{endpoint.address().to_string()};
  if (address_str == "0.0.0.0") {
    address_str = "localhost";
  }
  spdlog::info("start server at: http://{}:{}", address_str, endpoint.port());
  do_accept();
}

void server::run() {
  // The io_context::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  m_context.run();
}

void server::do_accept() {
  m_acceptor.async_accept(
      [this](std::error_code ec, asio::ip::tcp::socket socket) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!m_acceptor.is_open()) {
          return;
        }

        if (!ec) {
          m_connection_manager->start(std::make_shared<connection>(
              std::move(socket), m_connection_manager, m_request_handler));
        }

        do_accept();
      });
}

void server::do_await_stop() {
  m_signal_set.async_wait([this](std::error_code /*ec*/, int signo) {
    // The server is stopped by cancelling all outstanding asynchronous
    // operations. Once all operations have finished the io_context::run()
    // call will exit.
    spdlog::info("receive signal: {}, server exit!", signo);
    m_acceptor.close();
    m_connection_manager->stop_all();
  });
}

} // namespace server
} // namespace http
