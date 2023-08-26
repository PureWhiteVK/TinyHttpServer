#include "connection.hpp"
#include "connection_manager.hpp"
#include "response.hpp"

#include <spdlog/spdlog.h>

namespace http {
namespace server {

connection::connection(asio::ip::tcp::socket stream,
                       std::shared_ptr<connection_manager> manager,
                       std::shared_ptr<request_handler> handler)
    : base_connection(manager, handler), m_stream(std::move(stream)),
      m_timer(m_stream.get_executor()) {}

void connection::start() { do_read(); }

void connection::stop() {
  m_timer.cancel();
  asio::error_code err;
  m_stream.close(err);
}

void connection::shutdown() {
  asio::error_code err;
  // 由服务端主动关闭，如何让客户端主动关闭请求呢？
  // 发送一个 Connection: close 报文？
  m_stream.shutdown(asio::ip::tcp::socket::shutdown_both, err);
}

void connection::update_expire_time() {
  // m_expire_time = timer::clock_type::now() + TIMEOUT;
  // 重设超时时间
  m_timer.expires_after(TIMEOUT);
  m_timer.async_wait([this, self = shared_from_this()](asio::error_code err) {
    if (err) {
      return;
    }
    spdlog::info("timeout!");
    shutdown();
  });
}

void connection::do_read() {
  m_stream.async_read_some(asio::buffer(m_buffer),
                           [this, self = shared_from_this()](
                               asio::error_code err, size_t bytes_transferred) {
                             on_data_received(err, bytes_transferred);
                           });
  update_expire_time();
}

void connection::do_write() {
  m_stream.async_write_some(
      m_send_buffers, [this, self = shared_from_this()](
                          asio::error_code err, size_t bytes_transferred) {
        on_data_sent(err, bytes_transferred);
      });
  update_expire_time();
}

} // namespace server
} // namespace http
