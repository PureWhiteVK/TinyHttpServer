#include "request_parser.hpp"
#include "request.hpp"
#include "string_utils.hpp"

#include <unordered_set>

namespace http {
namespace server {

static constexpr uint8_t SP = ' ';
static constexpr uint8_t CR = '\r';
static constexpr uint8_t LF = '\n';

bool request_parser::is_tchar(uint8_t ch) {
  static std::unordered_set<uint8_t> tchar_set{'!',  '#', '$', '%', '&',
                                               '\'', '*', '+', '-', '.',
                                               '^',  '_', '`', '|', '~'};
  return std::isalnum(ch) || tchar_set.find(ch) != tchar_set.end();
}

std::tuple<request_parser::parse_result, size_t>
request_parser::parse(std::shared_ptr<request> req, std::string_view data) {
  parse_result result;
  size_t pos;
  for (pos = 0; pos < data.size(); pos++) {
    result = consume(req, data[pos]);
    if (result != CONTINUE) {
      break;
    }
  }
  return std::make_tuple(result, pos);
}

request_parser::parse_result
request_parser::consume(std::shared_ptr<request> req, uint8_t ch) {
  parse_result res{CONTINUE};
  switch (m_state) {
  case request_line: {
    if (is_tchar(ch)) {
      m_buffer << ch;
      m_state = method;
    } else {
      res = FAIL;
    }
    break;
  }
  case method: {
    if (ch == SP) {
      req->method = m_buffer.str();
      m_buffer.str("");
      m_state = request_target;
    } else if (is_tchar(ch)) {
      m_buffer << ch;
    } else {
      res = FAIL;
    }
    break;
  }
  case request_target: {
    if (ch == SP) {
      req->request_target = m_buffer.str();
      m_buffer.str("");
      m_state = http_version_h;
    } else if (!std::iscntrl(ch)) {
      m_buffer << ch;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_h: {
    if (ch == 'H') {
      m_state = http_version_t_0;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_t_0: {
    if (ch == 'T') {
      m_state = http_version_t_1;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_t_1: {
    if (ch == 'T') {
      m_state = http_version_p;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_p: {
    if (ch == 'P') {
      m_state = http_version_slash;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_slash: {
    if (ch == '/') {
      m_state = http_version_major;
      req->http_version_major = 0;
      req->http_version_minor = 0;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_major: {
    if (std::isdigit(ch)) {
      req->http_version_major = (ch - '0');
      m_state = http_version_dot;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_dot: {
    if (ch == '.') {
      m_state = http_version_minor;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_minor: {
    if (std::isdigit(ch)) {
      req->http_version_minor = (ch - '0');
      m_state = request_line_cr;
    } else {
      res = FAIL;
    }
    break;
  }
  case request_line_cr: {
    if (ch == CR) {
      m_state = request_line_lf;
    } else {
      res = FAIL;
    }
    break;
  }
  case request_line_lf: {
    if (ch == LF) {
      m_state = field_line;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_line: {
    if (ch == CR) {
      m_state = body_lf;
    } else if (is_tchar(ch)) {
      m_buffer << ch;
      m_state = field_name;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_name: {
    if (ch == ':') {
      m_last_field_name = m_buffer.str();
      m_buffer.str("");
      m_state = field_value;
    } else if (is_tchar(ch)) {
      m_buffer << ch;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_value: {
    if (ch == CR) {
      // we have to strip buffer_value here
      req->headers[m_last_field_name] = string_utils::trim(m_buffer.str());
      m_last_field_name.clear();
      m_buffer.str("");
      m_state = field_line_lf;
    } else if (is_field_vchar(ch)) {
      m_buffer << ch;
    } else if (std::isblank(ch)) {
      m_buffer << ch;
      m_state = field_value_ows;
    } else {
      res = FAIL;
    }
    break;
  }
  case field_value_ows: {
    if (ch == CR) {
      // we have to strip buffer value here
      req->headers[m_last_field_name] = string_utils::trim(m_buffer.str());
      m_last_field_name.clear();
      m_buffer.str("");
      m_state = field_line_lf;
    } else if (is_field_vchar(ch)) {
      m_buffer << ch;
      m_state = field_value;
    } else if (std::isblank(ch)) {
      // skip
    } else {
      res = FAIL;
    }
    break;
  }
  case field_line_lf: {
    if (ch == LF) {
      m_state = field_line;
    } else {
      res = FAIL;
    }
    break;
  }
  case body_lf: {
    res = (ch == LF) ? PASS : FAIL;
    break;
  }
  }
  return res;
}

} // namespace server
} // namespace http
