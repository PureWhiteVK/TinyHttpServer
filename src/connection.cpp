#include "connection.hpp"
#include "connection_manager.hpp"
#include "response.hpp"

#include <spdlog/spdlog.h>

namespace http {
namespace server {

connection::connection(asio::ip::tcp::socket socket,
                       std::shared_ptr<connection_manager> manager,
                       std::shared_ptr<request_handler> handler)
    : base_connection(manager, handler), m_socket(std::move(socket)) {}

void connection::start() { do_read(); }

void connection::stop() { m_socket.close(); }

void connection::shutdown() {
  asio::error_code ignored_ec;
  m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

void connection::do_read() {
  m_socket.async_read_some(asio::buffer(m_buffer),
                           [this, self = shared_from_this()](
                               std::error_code err, size_t bytes_transferred) {
                             on_data_received(err, bytes_transferred);
                           });
}

void connection::do_write() {
  m_socket.async_write_some(m_send_buffers,
                            [this, self = shared_from_this()](
                                std::error_code err, size_t bytes_transferred) {
                              on_data_sent(err, bytes_transferred);
                            });
}

} // namespace server
} // namespace http
