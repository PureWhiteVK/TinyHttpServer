#include "ssl_connection.hpp"
#include "connection_manager.hpp"
#include "response.hpp"

#include <spdlog/spdlog.h>

namespace http {
namespace server {

ssl_connection::ssl_connection(ssl_connection::ssl_stream stream,
                               std::shared_ptr<connection_manager> manager,
                               std::shared_ptr<request_handler> handler)
    : base_connection(manager, handler), m_stream(std::move(stream)) {}

void ssl_connection::start() { do_handshake(); }

void ssl_connection::stop() { m_stream.lowest_layer().close(); }

void ssl_connection::shutdown() {
  asio::error_code ignored_ec;
  m_stream.shutdown();
  m_stream.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both,
                                   ignored_ec);
}

void ssl_connection::do_handshake() {
  auto self(shared_from_this());
  m_stream.async_handshake(asio::ssl::stream_base::server,
                           [this, self = shared_from_this()](
                               std::error_code err) { on_handshaked(err); });
}

void ssl_connection::on_handshaked(std::error_code err) {
  if (err) {
    spdlog::error("error during handshake {}: {}", err.category().name(),
                  err.message());
    return;
  }
  do_read();
}

void ssl_connection::do_read() {
  m_stream.async_read_some(asio::buffer(m_buffer),
                           [this, self = shared_from_this()](
                               std::error_code err, size_t bytes_transferred) {
                             on_data_received(err, bytes_transferred);
                           });
}

void ssl_connection::do_write() {
  m_stream.async_write_some(m_send_buffers,
                            [this, self = shared_from_this()](
                                std::error_code err, size_t bytes_transferred) {
                              // 此处有可能发送数据不完全？
                              on_data_sent(err, bytes_transferred);
                            });
}

} // namespace server
} // namespace http
