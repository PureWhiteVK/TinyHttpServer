#include "string_utils.hpp"

#include <sstream>
#include <unordered_map>

#include <spdlog/fmt/fmt.h>

namespace http {
namespace server {

namespace string_utils {
static bool isblank(char ch) { return ch == 0x20 || ch == 0x9; }

std::string escaped(std::string_view data) {
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
      } else if (std::iscntrl(ch)) {
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

std::string lower(std::string_view s) {
  std::string out;
  out.resize(s.size());
  std::transform(s.begin(), s.end(), out.begin(), [](char ch) -> char {
    return (ch >= 'A' && ch <= 'Z') ? (ch - 'A' + 'a') : ch;
  });
  return out;
}

std::string_view trim(std::string_view data) {
  auto begin = std::find_if_not(data.begin(), data.end(), isblank);
  auto end = std::find_if_not(data.rbegin(), data.rend(), isblank);
  auto begin_index = begin - data.begin();
  auto end_index = data.size() - (end - data.rbegin()) - 1;
  return data.substr(begin_index, end_index - begin_index + 1);
}

size_t parse_ull(std::string_view data) noexcept {
  size_t res = 0;
  for (uint8_t ch : data) {
    if (std::isdigit(ch)) {
      res = res * 10 + (ch - '0');
    } else {
      res = 0;
      break;
    }
  }
  return res;
}

} // namespace string_utils
} // namespace server
} // namespace http