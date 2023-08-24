#pragma once

#include <spdlog/fmt/fmt.h>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>


namespace http {
namespace server {
inline static std::string lower(std::string_view s) {
  std::stringstream stream;
  for (char ch : s) {
    if (ch >= 'A' && ch <= 'Z') {
      stream << static_cast<char>((ch - 'A') + 'a');
    } else {
      stream << static_cast<char>(ch);
    }
  }
  return stream.str();
}

inline static std::string escaped(std::string_view data) {
  // from https://en.cppreference.com/w/cpp/language/escape
  static std::unordered_map<char, std::string_view> escaped_char{
      {'\'', "\\'"}, {'\"', "\\\""}, {'\?', "\\\?"}, {'\\', "\\\\"},
      {'\a', "\\a"}, {'\b', "\\b"},  {'\f', "\\f"},  {'\n', "\\n"},
      {'\r', "\\r"}, {'\t', "\\t"},  {'\v', "\\v"},
  };
  std::stringstream s;
  s << 'b' << '\'';
  for (uint8_t ch : data) {
    // 判断 ch 是否是可见字符？
    if (ch < 128) {
      // ascii
      if (escaped_char.find(ch) != escaped_char.end()) {
        s << escaped_char[ch];
      } else if (ch < 32 || ch == 127) {
        s << fmt::format("\\x{:02x}", ch);
      } else {
        s << ch;
      }
    } else {
      // hex output
      s << fmt::format("\\x{:02x}", ch);
    }
  }
  s << '\'';
  return s.str();
}

} // namespace server
} // namespace http