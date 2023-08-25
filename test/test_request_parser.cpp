#include "request.hpp"
#include "request_parser.hpp"
#include "string_utils.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <spdlog/spdlog.h>

template <typename T> static std::string_view from_buffer(const T &t) {
  return {t.data(), t.size()};
}

using namespace http::server;
namespace fs = std::filesystem;
int main() {
  fs::path data_path = fs::u8path(DATA_PATH);
  std::ifstream request_data(data_path / "test_post3.txt",
                             std::ios::in | std::ios::binary);
  std::array<char, 64> buffer;
  std::shared_ptr<request> req = std::make_shared<request>();
  request_parser parser;
  bool receive_body = false;
  size_t received_data_size = 0;
  while (!request_data.eof()) {
    request_data.read(buffer.data(), buffer.size());
    std::string_view data(buffer.data(), request_data.gcount());
    spdlog::info("receive data: {}", string_utils::escaped(data));
    if (!receive_body) {
      auto [parse_result, pos] = parser.parse(req, data);
      switch (parse_result) {
      case request_parser::PASS: {
        spdlog::info("finish parsing,pos: {}", pos);
        if (pos < data.size() - 1) {
          receive_body = true;
          received_data_size += data.size() - pos - 1;
        }
        break;
      }
      case request_parser::FAIL: {
        spdlog::error("failed to parse!");
        exit(EXIT_FAILURE);
      }
      case request_parser::CONTINUE:
        break;
      }
    } else {
      received_data_size += data.size();
    }
  }
  req->update();
  spdlog::info("receive http body: {} bytes, expect size: {} bytes",
               received_data_size, req->content_length);
  return 0;
}