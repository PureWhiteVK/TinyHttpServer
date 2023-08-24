#pragma once

#include <array>
#include <asio.hpp>
#include <memory>

#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "response.hpp"

namespace http {
namespace server {

class connection_manager;
class response;

/// Represents a single connection from a client.
class connection : public std::enable_shared_from_this<connection> {
public:
  connection(const connection &) = delete;
  connection &operator=(const connection &) = delete;

  /// Construct a connection with the given socket.
  explicit connection(asio::ip::tcp::socket socket,
                      std::shared_ptr<connection_manager> manager,
                      std::shared_ptr<request_handler> handler)
      : m_socket(std::move(socket)), m_connection_manager(manager),
        m_handler(handler) {}

  /// Start the first asynchronous operation for the connection.
  void start();

  /// Stop all asynchronous operations associated with the connection.
  void stop();

private:
  /// Perform an asynchronous read operation.
  void do_read();

  /// Perform an asynchronous write operation.
  void do_write(std::shared_ptr<response> response);

  /// Socket for the connection.
  asio::ip::tcp::socket m_socket;

  /// The manager for this connection.
  std::shared_ptr<connection_manager> m_connection_manager;

  // /// The handler used to process the incoming request.
  std::shared_ptr<request_handler> m_handler;

  /// Buffer for incoming data.
  std::array<char, 8192> m_buffer;

  // temporary data buffer(handle tcp stream problem)
  std::array<char, 1024> m_databuffer;

  /// The incoming request.
  request m_request;

  /// The parser for the incoming request.
  request_parser m_request_parser;

  /// The reply to be sent back to the client.
  // response m_response;

  /// keep alive
  bool m_keep_alive;
};

using connection_ptr = std::shared_ptr<connection>;

} // namespace server
} // namespace http
