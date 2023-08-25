#pragma once

#include "base_connection.hpp"
#include <array>
#include <asio.hpp>
#include <memory>

namespace http {
namespace server {

class connection : public base_connection {
public:
  static std::shared_ptr<connection>
  create(asio::ip::tcp::socket socket,
         std::shared_ptr<connection_manager> manager,
         std::shared_ptr<request_handler> handler) {
    return std::shared_ptr<connection>(
        new connection(std::move(socket), manager, handler));
  }
  virtual void start();
  virtual void stop();
  virtual void shutdown();

protected:
  explicit connection(asio::ip::tcp::socket socket,
                      std::shared_ptr<connection_manager> manager,
                      std::shared_ptr<request_handler> handler);

  virtual void do_read();
  virtual void do_write();

protected:
  asio::ip::tcp::socket m_socket;
};

} // namespace server
} // namespace http
