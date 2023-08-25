#pragma once

#include <string>
#include <unordered_map>

namespace http {
namespace server {

/// A request received from a client.
struct request {
public:
  std::string method{};
  // this may be parsed?
  std::string request_target{};
  int http_version_major{};
  int http_version_minor{};
  std::unordered_map<std::string, std::string> headers{};
  bool keep_alive{};
  size_t content_length{};

  void update() {
    keep_alive = get_keep_alive();
    content_length = get_content_length();
  }

  void clear() { headers.clear(); }

private:
  bool get_keep_alive();

  size_t get_content_length();
};

} // namespace server
} // namespace http
