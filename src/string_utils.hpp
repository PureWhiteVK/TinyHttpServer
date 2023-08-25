#pragma once

#include <algorithm>
#include <string>
#include <string_view>

namespace http {
namespace server {

namespace string_utils {
std::string lower(std::string_view s);

std::string_view trim(std::string_view data);

std::string escaped(std::string_view data);

size_t parse_ull(std::string_view data) noexcept;
} // namespace string_utils
} // namespace server
} // namespace http