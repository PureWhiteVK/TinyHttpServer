#pragma once

#include "base_connection.hpp"
#include <array>
#include <asio.hpp>
#include <memory>

namespace http {
namespace server {

class connection : public base_connection {
public:
  using timer = asio::steady_timer;
  using stream = asio::ip::tcp::socket;
  static std::shared_ptr<connection>
  create(stream stream,
         std::shared_ptr<connection_manager> manager,
         std::shared_ptr<request_handler> handler) {
    return std::shared_ptr<connection>(
        new connection(std::move(stream), manager, handler));
  }
  virtual void start();
  virtual void stop();
  virtual void shutdown();

protected:
  explicit connection(stream stream,
                      std::shared_ptr<connection_manager> manager,
                      std::shared_ptr<request_handler> handler);

  virtual void do_read();
  virtual void do_write();

  void update_expire_time();

protected:
  stream m_stream;
  timer m_timer;
  // timer::time_point m_expire_time{};
  // 无连接 15s 后关闭连接
  static constexpr auto TIMEOUT = std::chrono::seconds(15);
};

} // namespace server
} // namespace http
