#pragma once

#include <string>
#include <string_view>

namespace http {
namespace server {
namespace mime_types {

/// Convert a file extension into a MIME type.
std::string_view extension_to_type(const std::string& extension);

} // namespace mime_types
} // namespace server
} // namespace http
