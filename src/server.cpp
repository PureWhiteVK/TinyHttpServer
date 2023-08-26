#include "server.hpp"
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include "ssl_connection.hpp"

#include <iostream>
#include <signal.h>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace http {
namespace server {

server::server(std::string_view address, std::string_view port,
               const fs::path &doc_root, const fs::path &cert_path,
               const fs::path &key_path)
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

  m_signal_set.async_wait([this](std::error_code /*ec*/, int signo) {
    // The server is stopped by cancelling all outstanding asynchronous
    // operations. Once all operations have finished the io_context::run()
    // call will exit.
    spdlog::info("receive signal: {}, server exit!", signo);
    m_acceptor.close();
    m_connection_manager->stop_all();
  });

  static constexpr bool enable_ssl = true;

  if (enable_ssl && fs::exists(cert_path) && fs::exists(key_path)) {
    spdlog::info("enable SSL/TLS");
    // sslv23 means generic SSL/TLS (support both)
    // SSLv2 is too old and insecure, so we disable support for it!
    m_ssl_context = asio::ssl::context(asio::ssl::context::sslv23);
    m_ssl_context->set_options(asio::ssl::context::default_workarounds);
    asio::error_code err;
    m_ssl_context->use_certificate_file(cert_path.u8string().c_str(),
                                        asio::ssl::context::file_format::pem,
                                        err);
    if (err) {
      throw std::runtime_error(
          fmt::format("[file: {},line: {}] {}: {}", __FILE__, __LINE__,
                      err.category().name(), err.message()));
    }
    m_ssl_context->use_private_key_file(
        key_path.u8string().c_str(), asio::ssl::context::file_format::pem, err);
    if (err) {
      throw std::runtime_error(
          fmt::format("[file: {},line: {}] {}: {}", __FILE__, __LINE__,
                      err.category().name(), err.message()));
    }
  }

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  asio::ip::tcp::resolver resolver(m_context);
  auto endpoints = resolver.resolve(address, port);
  for (auto e : endpoints) {
    spdlog::info("resolved: {}://{}:{}", m_ssl_context ? "https" : "http",
                 e.endpoint().address().to_string(), e.endpoint().port());
  }
  asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
  m_acceptor.open(endpoint.protocol());
  // first true means enable linger, the second means timeout value
  // m_acceptor.set_option(asio::ip::tcp::acceptor::linger(true, 30));
  m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  m_acceptor.bind(endpoint);
  m_acceptor.listen();
  std::string address_str = endpoint.address().to_string();
  if (address_str == "0.0.0.0") {
    address_str = "localhost";
  }
  spdlog::info("start server at: {}://{}:{}", m_ssl_context ? "https" : "http",
               address_str, endpoint.port());
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
      [this](asio::error_code ec, asio::ip::tcp::socket socket) {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!m_acceptor.is_open()) {
          return;
        }

        if (ec) {
          spdlog::error("[file:{},line:{}] {}: {}", __FILE__, __LINE__,
                        ec.category().name(), ec.message());
          return;
        }

        spdlog::info("connected: {}:{}",
                     socket.remote_endpoint().address().to_string(),
                     socket.remote_endpoint().port());

        if (m_ssl_context) {
          m_connection_manager->start(ssl_connection::create(
              ssl_connection::stream(std::move(socket), m_ssl_context.value()),
              m_connection_manager, m_request_handler));
        } else {
          m_connection_manager->start(connection::create(
              std::move(socket), m_connection_manager, m_request_handler));
        }

        do_accept();
      });
}

} // namespace server
} // namespace http
