#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>

namespace http {
namespace server {

struct request;

/// Parser for incoming requests.
class request_parser {
public:
  /// Construct ready to parse the request method.
  request_parser() = default;

  /// Reset to initial parser state.
  void reset() { m_state = parser_state::request_line; }

  /// Result of parse.
  enum parse_result { PASS, FAIL, CONTINUE };

  /// Parse some data. The enum return value is good when a complete request has
  /// been parsed, bad if the data is invalid, indeterminate when more data is
  /// required. The InputIterator return value indicates how much of the input
  /// has been consumed.
  template <typename InputIterator>
  std::tuple<parse_result, InputIterator>
  parse(request &req, InputIterator begin, InputIterator end) {
    parse_result result;
    while (begin != end) {
      result = consume(req, *begin++);
      if (result != CONTINUE) {
        break;
      }
    }
    return std::make_tuple(result, begin);
  }

private:
  /// Handle the next character of input.
  parse_result consume(request &req, uint8_t input);

  static bool is_digit(uint8_t ch) { return ch >= '0' && ch <= '9'; }

  static bool is_alpha(uint8_t ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
  }

  static bool is_obs_text(uint8_t ch) { return ch >= 0x80; }

  static bool is_vchar(uint8_t ch) { return ch > 0x20 && ch <= 0x7e; }

  static bool is_ctl(uint8_t ch) { return ch <= 0x20 || ch == 0x7f; }

  static bool is_tchar(uint8_t ch) {
    static std::unordered_set<uint8_t> tchar_set{'!',  '#', '$', '%', '&',
                                                 '\'', '*', '+', '-', '.',
                                                 '^',  '_', '`', '|', '~'};
    return is_digit(ch) || is_alpha(ch) ||
           tchar_set.find(ch) != tchar_set.end();
  }

  static bool is_field_vchar(uint8_t ch) {
    return is_vchar(ch) || is_obs_text(ch);
  }

  static std::string_view trim(std::string_view data) {
    auto begin = std::find_if_not(data.begin(), data.end(), [](uint8_t ch) {
      return ch == SP || ch == HTAB;
    });
    auto end = std::find_if_not(data.rbegin(), data.rend(), [](uint8_t ch) {
      return ch == SP || ch == HTAB;
    });
    auto begin_index = begin - data.begin();
    auto end_index = data.size() - (end - data.rbegin()) - 1;
    return data.substr(begin_index, end_index - begin_index + 1);
  }

  static constexpr uint8_t SP = ' ';
  static constexpr uint8_t HTAB = '\t';
  static constexpr uint8_t CR = '\r';
  static constexpr uint8_t LF = '\n';

  /// The current state of the parser.
  enum parser_state {
    request_line,
    method,
    request_target,
    http_version_h,
    http_version_t_0,
    http_version_t_1,
    http_version_p,
    http_version_slash,
    http_version_major,
    http_version_dot,
    http_version_minor,
    request_line_cr,
    request_line_lf,
    field_line,
    field_name,
    field_value,
    field_value_ows,
    field_line_lf,
    body_lf,
  } m_state{request_line};

  std::ostringstream m_buffer;
  std::string m_last_field_name;
};

} // namespace server
} // namespace http

