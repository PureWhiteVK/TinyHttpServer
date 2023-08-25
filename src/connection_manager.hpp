#pragma once

#include <memory>
#include <unordered_set>

namespace http {
namespace server {

class base_connection;

class connection_manager {
private:
  using connection_ptr = std::shared_ptr<base_connection>;

public:
  connection_manager(const connection_manager &) = delete;
  connection_manager &operator=(const connection_manager &) = delete;

  connection_manager();

  void start(connection_ptr c);

  void stop(connection_ptr c);

  void stop_all();

private:
  std::unordered_set<connection_ptr> m_connections;
};

} // namespace server
} // namespace http