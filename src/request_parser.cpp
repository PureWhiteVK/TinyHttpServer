#include "request_parser.hpp"
#include "request.hpp"

namespace http {
namespace server {

request_parser::parse_result request_parser::consume(request &req, uint8_t ch) {
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
      req.method = m_buffer.str();
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
      req.request_target = m_buffer.str();
      m_buffer.str("");
      m_state = http_version_h;
    } else if (!is_ctl(ch)) {
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
      req.http_version_major = 0;
      req.http_version_minor = 0;
    } else {
      res = FAIL;
    }
    break;
  }
  case http_version_major: {
    if (is_digit(ch)) {
      req.http_version_major = (ch - '0');
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
    if (is_digit(ch)) {
      req.http_version_minor = (ch - '0');
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
      req.headers[m_last_field_name] = trim(m_buffer.str());
      m_last_field_name.clear();
      m_buffer.str("");
      m_state = field_line_lf;
    } else if (is_field_vchar(ch)) {
      m_buffer << ch;
    } else if (ch == SP || ch == HTAB) {
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
      req.headers[m_last_field_name] = trim(m_buffer.str());
      m_last_field_name.clear();
      m_buffer.str("");
      m_state = field_line_lf;
    } else if (is_field_vchar(ch)) {
      m_buffer << ch;
      m_state = field_value;
    } else if (ch == SP || ch == HTAB) {
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
