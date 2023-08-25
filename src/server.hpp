#pragma once

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <filesystem>
#include <optional>
#include <string>

namespace http {
namespace server {

class connection_manager;
class request_handler;

/// The top-level class of the HTTP server.
class server {
public:
  server(const server &) = delete;
  server &operator=(const server &) = delete;

  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit server(std::string_view address, std::string_view port,
                  const std::filesystem::path &doc_root,
                  const std::filesystem::path &cert_path =
                      std::filesystem::path(DATA_PATH) / "CA/cert.pem",
                  const std::filesystem::path &key_path =
                      std::filesystem::path(DATA_PATH) / "CA/key.pem");

  /// Run the server's io_context loop.
  void run();

private:
  /// Perform an asynchronous accept operation.
  void do_accept();

  /// The io_context used to perform asynchronous operations.
  asio::io_context m_context;

  /// The signal_set is used to register for process termination notifications.
  asio::signal_set m_signal_set;

  /// Acceptor used to listen for incoming connections.
  asio::ip::tcp::acceptor m_acceptor;

  /// The connection manager which owns all live connections.
  std::shared_ptr<connection_manager> m_connection_manager;

  /// The handler for all incoming requests.
  std::shared_ptr<request_handler> m_request_handler;

  /// ssl context
  std::optional<asio::ssl::context> m_ssl_context;
};

} // namespace server
} // namespace http
