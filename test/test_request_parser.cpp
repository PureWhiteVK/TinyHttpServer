#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "request.hpp"
#include "request_parser.hpp"

namespace fs = std::filesystem;

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

bool is_digit(uint8_t ch) { return ch >= '0' && ch <= '9'; }

bool is_alpha(uint8_t ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool is_obs_text(uint8_t ch) { return ch >= 0x80; }

bool is_vchar(uint8_t ch) { return ch > 0x20 && ch <= 0x7e; }

bool is_ctl(uint8_t ch) { return ch <= 0x20 || ch == 0x7f; }

bool is_tchar(uint8_t ch) {
  static std::unordered_set<uint8_t> tchar_set{'!',  '#', '$', '%', '&',
                                               '\'', '*', '+', '-', '.',
                                               '^',  '_', '`', '|', '~'};
  return is_digit(ch) || is_alpha(ch) || tchar_set.find(ch) != tchar_set.end();
}

bool is_field_vchar(uint8_t ch) { return is_vchar(ch) || is_obs_text(ch); }

static constexpr uint8_t SP = ' ';
static constexpr uint8_t HTAB = '\t';
static constexpr uint8_t CR = '\r';
static constexpr uint8_t LF = '\n';

enum parse_result { PASS, FAIL, CONTINUE };

struct request {
  std::string method;
  std::string request_target;
  int http_version_major;
  int http_version_minor;
  std::unordered_map<std::string, std::string> headers;
};

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

class request_parser {
public:
  request_parser() = default;

  void reset() { m_state = request_line; }

  std::pair<parse_result, size_t> parse(request &req, std::string_view data) {
    parse_result res{PASS};
    char ch;
    size_t pos = 0;
    for (; pos < data.size(); pos++) {
      res = consume(req, data[pos]);
      if (res != CONTINUE) {
        break;
      }
    }
    return {res, pos};
  }

  parse_result consume(request &req, uint8_t ch) {
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
        // spdlog::info("request_target: {}", m_buffer.str());
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
        // spdlog::info("http version major: {}", (ch - '0'));
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
        // spdlog::info("http version minor: {}", (ch - '0'));
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
        last_field_name = m_buffer.str();
        // spdlog::info("header field name: {}", escaped(m_buffer.str()));
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
        req.headers[last_field_name] = trim(m_buffer.str());
        last_field_name.clear();
        // spdlog::info("header field value: {}",
        // escaped(trim(m_buffer.str())));
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
        req.headers[last_field_name] = trim(m_buffer.str());
        last_field_name.clear();
        // spdlog::info("header field value: {}",
        // escaped(trim(m_buffer.str())));
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

private:
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
  std::string last_field_name;
};

int main() {
  try {
    spdlog::info("data_path: {}", DATA_PATH);
    fs::path data_path(DATA_PATH);
    fs::path request_test_path = data_path / "test_post2.txt";
    std::ifstream stream(request_test_path, std::ios::in | std::ios::binary);
    std::array<char, 64> buffer;
    size_t total_size = 0;
    request req;
    request_parser parser;
    bool get_request_body = false;
    size_t body_size = 0;
    while (!stream.eof()) {
      stream.read(buffer.data(), buffer.size());
      std::string_view curr_read(buffer.data(), stream.gcount());
      spdlog::info("receive data: {}, read_size: {}", escaped(curr_read),
                   stream.gcount());
      total_size += stream.gcount();
      if (!get_request_body) {
        auto [res, pos] = parser.parse(req, curr_read);
        if (res == FAIL) {
          throw std::runtime_error("parse failed!");
        } else if (res == PASS) {
          spdlog::info("http header parse finished!");
          // the rest data is body
          if (pos < curr_read.size() - 1) {
            get_request_body = true;
            // body data starts from pos + 1
            body_size += curr_read.size() - (pos + 1);
            spdlog::info("contains body, size: {}", curr_read.size() - pos);
          }
        }
      } else {
        // loading body
        body_size += curr_read.size();
      }

      // auto [res, pos] =
      //     parser.parse(request, curr_read.begin(), curr_read.end());
      // 然后此处结合读取 content-size 读取请求包大小
      // if (pos != curr_read.end()) {
      //   spdlog::info("unfinished yet, contains body ??");
      // }
      // if (res == parser.bad) {
      //   throw std::runtime_error("parse failed!");
      // }
      // spdlog::info("parse result: {}", res);
    }
    spdlog::info("total size: {}", total_size);
    spdlog::info("body size: {}", body_size);
    spdlog::info("{} {} HTTP/{}.{}", req.method, req.request_target,
                 req.http_version_major, req.http_version_minor);

  } catch (const std::exception &e) {
    spdlog::error("{}", e.what());
  }
  return 0;
}