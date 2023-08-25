#include "connection_manager.hpp"
#include "base_connection.hpp"
#include <spdlog/spdlog.h>

namespace http {
namespace server {

connection_manager::connection_manager() {}

void connection_manager::start(connection_ptr c) {
  m_connections.insert(c);
  c->start();
}

void connection_manager::stop(connection_ptr c) {
  m_connections.erase(c);
  c->stop();
}

void connection_manager::stop_all() {
  spdlog::info("close {} connections", m_connections.size());
  for (auto c : m_connections) {
    c->stop();
  }
  m_connections.clear();
}

} // namespace server
} // namespace http
