#include "base_connection.hpp"
#include "connection_manager.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "response.hpp"
#include "string_utils.hpp"

#include <spdlog/spdlog.h>

namespace http {
namespace server {
base_connection::base_connection(std::shared_ptr<connection_manager> manager,
                                 std::shared_ptr<request_handler> handler)
    : m_manager(manager), m_handler(handler),
      m_parser(std::make_shared<request_parser>()),
      m_request(std::make_shared<request>()),
      m_response(std::make_shared<response>()) {}

void base_connection::on_data_received(asio::error_code err,
                                       size_t bytes_transferred) {
  if (err) {
    spdlog::error("[{}:{}] {}: {}", __FILE__, __LINE__, err.category().name(),
                  err.message());
    m_manager->stop(shared_from_this());
    return;
  }
  // spdlog::info("receive data: {} bytes", bytes_transferred);
  auto [parse_result, pos] = m_parser->parse(
      m_request, std::string_view(m_buffer.data(), bytes_transferred));
  switch (parse_result) {
  case request_parser::PASS: {
    m_request->update();
    m_keep_alive = m_request->keep_alive;
    // if (m_request->content_length > 0) {
    //   spdlog::info("content length: {}", m_request->content_length);
    // }
    // spdlog::info("pos: {}, bytes_transferred: {}", pos, bytes_transferred);
    m_handler->handle_request(m_request, m_response);
    get_send_buffers();
    do_write();
    break;
  }
  case request_parser::FAIL: {
    spdlog::error("http header parse failed");
    spdlog::error("request:\n{}", string_utils::escaped(std::string_view(
                                      m_buffer.data(), bytes_transferred)));
    response::build_default_response(m_response, response::bad_request);
    get_send_buffers();
    do_write();
    break;
  }
  case request_parser::CONTINUE: {
    do_read();
    break;
  }
  }
}

void base_connection::get_send_buffers() {
  m_response->update(m_keep_alive);
  m_response->to_buffers(m_send_buffers);
  std::string_view response_header =
      reinterpret_cast<const char *>(m_send_buffers.front().data());
  spdlog::info("response: {}",
               response_header.substr(0, response_header.find('\r')));
}

void base_connection::clear() {
  m_parser->clear();
  m_request->clear();
  m_response->clear();
}

void base_connection::on_data_sent(asio::error_code err,
                                   size_t bytes_transferred) {
  if (err) {
    spdlog::error("[{}:{}] {}: {}", __FILE__, __LINE__, err.category().name(),
                  err.message());
    m_manager->stop(shared_from_this());
    return;
  }
  spdlog::info("send: {} bytes", bytes_transferred);
  while (!m_send_buffers.empty() &&
         m_send_buffers.front().size() <= bytes_transferred) {
    bytes_transferred -= m_send_buffers.front().size();
    m_send_buffers.pop_front();
  }
  if (m_send_buffers.empty()) {
    spdlog::info("finish sending");
    if (!m_keep_alive) {
      // spdlog::info("no keep-alive, close connection!", bytes_transferred);
      shutdown();
      return;
    }
    clear();
    do_read();
    return;
  }
  // 之后我们可以计算剩余的偏移量
  m_send_buffers.front() = {
      reinterpret_cast<const void *>(
          reinterpret_cast<const uint8_t *>(m_send_buffers.front().data()) +
          bytes_transferred),
      m_send_buffers.front().size() - bytes_transferred};
  do_write();
}
} // namespace server
} // namespace http