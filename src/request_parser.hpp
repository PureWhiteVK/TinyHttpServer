#pragma once

#include <cctype>
#include <sstream>
#include <string>
#include <tuple>

namespace http {
namespace server {

struct request;

/// Parser for incoming requests.
class request_parser {
public:
  /// Construct ready to parse the request method.
  request_parser() = default;

  /// Reset to initial parser state.
  void clear() {
    m_state = parser_state::request_line;
    m_buffer.str("");
  }

  /// Result of parse.
  enum parse_result { PASS, FAIL, CONTINUE };

  /// Parse some data. The enum return value is good when a complete request has
  /// been parsed, bad if the data is invalid, indeterminate when more data is
  /// required. The InputIterator return value indicates how much of the input
  /// has been consumed.
  std::tuple<parse_result, size_t> parse(std::shared_ptr<request> req,
                                         std::string_view data);

private:
  /// Handle the next character of input.
  parse_result consume(std::shared_ptr<request> req, uint8_t input);

  static bool is_obs_text(uint8_t ch) { return ch >= 0x80; }

  static bool is_vchar(uint8_t ch) { return ch > 0x20 && ch <= 0x7e; }

  static bool is_tchar(uint8_t ch);

  static bool is_field_vchar(uint8_t ch) {
    return is_vchar(ch) || is_obs_text(ch);
  }

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
