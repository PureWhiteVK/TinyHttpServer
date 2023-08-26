#pragma once
#include <array>
#include <asio.hpp>
#include <memory>
#include <list>

namespace http {
namespace server {
class connection_manager;
class request_handler;
class request_parser;
struct request;
struct response;

class base_connection : public std::enable_shared_from_this<base_connection> {
public:
  base_connection(const base_connection &) = delete;
  base_connection &operator=(const base_connection &) = delete;

  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void shutdown() = 0;

protected:
  explicit base_connection(std::shared_ptr<connection_manager> manager,
                           std::shared_ptr<request_handler> handler);

  virtual void do_read() = 0;
  virtual void do_write() = 0;

  void clear();

  void get_send_buffers();

  void on_data_received(asio::error_code err, size_t bytes_transferred);
  void on_data_sent(asio::error_code err, size_t bytes_transferred);

protected:
  std::shared_ptr<connection_manager> m_manager{};
  std::shared_ptr<request_handler> m_handler{};
  std::shared_ptr<request_parser> m_parser{};

  // receive buffer
  std::array<char, 8192> m_buffer{};

  // for o(1) push_back and o(1) pop_front and begin()/end() iteration
  std::list<asio::const_buffer> m_send_buffers{};

  std::shared_ptr<request> m_request{};
  std::shared_ptr<response> m_response{};
  bool m_keep_alive{};
};

using connection_ptr = std::shared_ptr<base_connection>;

} // namespace server
} // namespace http