#include "ssl_connection.hpp"
#include "connection_manager.hpp"
#include "response.hpp"

#include <spdlog/spdlog.h>

namespace http {
namespace server {

ssl_connection::ssl_connection(ssl_connection::stream stream,
                               std::shared_ptr<connection_manager> manager,
                               std::shared_ptr<request_handler> handler)
    : base_connection(manager, handler), m_stream(std::move(stream)),
      m_timer(m_stream.get_executor()) {}

void ssl_connection::start() { do_handshake(); }

void ssl_connection::stop() {
  m_timer.cancel();
  asio::error_code err;
  // close 就会强制关闭（也就是 RST报文）
  m_stream.lowest_layer().close(err);
}

// error from: https://www.cnblogs.com/langtianya/p/6648100.html
// https://ask.wireshark.org/question/10060/keep-alive-packets-after-fin/

void ssl_connection::shutdown() {
  // 此处 shutdown 会让客户端主动关闭连接，由于会发送连接，需要异步 shutdown
  spdlog::info("perform async SSL shutdown");
  m_timer.expires_after(std::chrono::seconds(30));
  m_timer.async_wait([this, self = shared_from_this()](asio::error_code err) {
    if (err) {
      return;
    }
    spdlog::warn("SSL shutdown timeout after 30 seconds, stop right now!");
    // asio::error_code err;
    m_stream.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, err);
    spdlog::warn("shutdown {}: {}", err.category().name(), err.message());
    // m_manager->stop(self);
  });
  m_stream.async_shutdown(
      [this, self = shared_from_this()](asio::error_code /* err */) {
        // spdlog::warn("shutdown {}: {}", err.category().name(),
        // err.message());
      });
  // asio::error_code err;
  // m_stream.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both,
  // err); spdlog::warn("shutdown {}: {}", err.category().name(),
  // err.message());
}

void ssl_connection::do_handshake() {
  auto self(shared_from_this());
  m_stream.async_handshake(asio::ssl::stream_base::server,
                           [this, self = shared_from_this()](
                               asio::error_code err) { on_handshaked(err); });
}

void ssl_connection::on_handshaked(std::error_code err) {
  if (err) {
    spdlog::error("error during handshake {}: {}", err.category().name(),
                  err.message());
    m_manager->stop(shared_from_this());
    return;
  }
  // 设置 expire_time
  do_read();
}

void ssl_connection::update_expire_time() {
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

void ssl_connection::do_read() {
  m_stream.async_read_some(asio::buffer(m_buffer),
                           [this, self = shared_from_this()](
                               asio::error_code err, size_t bytes_transferred) {
                             on_data_received(err, bytes_transferred);
                           });
  update_expire_time();
}

void ssl_connection::do_write() {
  m_stream.async_write_some(
      m_send_buffers, [this, self = shared_from_this()](
                          asio::error_code err, size_t bytes_transferred) {
        on_data_sent(err, bytes_transferred);
      });
  update_expire_time();
}

} // namespace server
} // namespace http
