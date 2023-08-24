#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/spdlog.h>
#include <vector>

#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include "string_utils.hpp"

namespace http {
namespace server {

void connection::start() {
  spdlog::info("connection established, remote endpoint {}:{}",
               m_socket.remote_endpoint().address().to_string(),
               m_socket.remote_endpoint().port());
  do_read();
}

void connection::stop() { m_socket.close(); }

void connection::do_read() {
  m_socket.async_read_some(
      asio::buffer(m_buffer),
      [this, _ = shared_from_this()](std::error_code ec,
                                     std::size_t bytes_transferred) {
        if (!ec) {
          spdlog::info("receive data: {} bytes",bytes_transferred);
          auto [result, pos] =
              m_request_parser.parse(m_request, m_buffer.begin(),
                                     m_buffer.begin() + bytes_transferred);
          if (result == request_parser::PASS) {
            m_request.post_parsing();
            m_keep_alive = m_request.keep_alive;
            if(m_request.content_length > 0) {
              spdlog::info("content length: {}",m_request.content_length);
            }
            spdlog::info("pos: {}, bytes_transferred: {}",
                         pos - m_buffer.begin(), bytes_transferred);
            
            m_request_parser.reset();
            auto rep = std::make_shared<response>();
            m_handler->handle_request(m_request, rep, m_keep_alive);
            do_write(rep);
          } else if (result == request_parser::FAIL) {
            auto rep = std::make_shared<response>();
            response::build_default_response(rep, response::bad_request);
            do_write(rep);
          }
        } else if (ec != asio::error::operation_aborted) {
          m_connection_manager->stop(shared_from_this());
          return;
        }
        do_read();
      });
}

void connection::do_write(std::shared_ptr<response> rep) {
  rep->headers["Content-Length"] = std::to_string(rep->content.size());
  rep->headers["Connection"] = m_keep_alive ? "keep-alive" : "close";
  auto buffers = rep->to_buffers();
  std::string_view response_header =
      reinterpret_cast<const char *>(buffers[0].data());
  spdlog::info("response: {}",
               response_header.substr(0, response_header.find('\r')));
  asio::async_write(m_socket, buffers,
                    [this, _ = shared_from_this(),
                     rep](std::error_code ec, std::size_t bytes_transferred) {
                      // 从不主动关闭连接？
                      if (!ec) {
                        if (!m_keep_alive) {
                          spdlog::info("write {} byte(s), close connection!",
                                       bytes_transferred);
                          // Initiate graceful connection closure.
                          asio::error_code ignored_ec;
                          m_socket.shutdown(
                              asio::ip::tcp::socket::shutdown_both, ignored_ec);
                        }
                      } else if (ec != asio::error::operation_aborted) {
                        m_connection_manager->stop(shared_from_this());
                      }
                    });
}

} // namespace server
} // namespace http
