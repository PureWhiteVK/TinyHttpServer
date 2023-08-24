#include "mime_types.hpp"

#include <string_view>
#include <unordered_map>

namespace http {
namespace server {
namespace mime_types {

std::string_view extension_to_type(const std::string& extension) {
  static std::unordered_map<std::string, std::string> mappings{
      {"gif", "image/gif"},    {"htm", "text/html"},
      {"html", "text/html"},   {"jpg", "image/jpeg"},
      {"png", "image/png"},    {"js", "text/javascript"},
      {"css", "text/css"},     {"ico", "image/vnd.microsoft.icon"},
      {"svg", "image/svg+xml"}};
  if (mappings.find(extension) != mappings.end()) {
    return mappings[extension];
  }
  return "text/plain";
}

} // namespace mime_types
} // namespace server
} // namespace http
