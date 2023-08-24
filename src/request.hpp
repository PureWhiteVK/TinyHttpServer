#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <unordered_map>

#include "string_utils.hpp"

namespace http {
namespace server {

/// A request received from a client.
class request {
public:
  std::string method{};
  std::string request_target{};
  int http_version_major{};
  int http_version_minor{};
  std::unordered_map<std::string, std::string> headers{};
  bool keep_alive{};
  size_t content_length{};

  void post_parsing() {
    keep_alive = get_keep_alive();
    content_length = get_content_length();
  }

private:
  bool get_keep_alive() {
    return headers.find("Connection") != headers.end() &&
           lower(headers["Connection"]) == "keep-alive";
  }

  size_t get_content_length() {
    return headers.find("Content-Length") == headers.end()
               ? 0
               : std::stoull(headers["Content-Length"]);
  }
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HPP
