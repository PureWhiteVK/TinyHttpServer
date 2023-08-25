#include "request.hpp"
#include "string_utils.hpp"

namespace http {
namespace server {
bool request::get_keep_alive() {
  return headers.find("Connection") != headers.end() &&
         string_utils::lower(headers["Connection"]) == "keep-alive";
}

size_t request::get_content_length() {
  return headers.find("Content-Length") == headers.end()
             ? 0
             : string_utils::parse_ull(headers["Content-Length"]);
}
} // namespace server
} // namespace http