#include "request_handler.hpp"
#include "mime_types.hpp"
#include "request.hpp"
#include "response.hpp"

#include <fstream>
#include <spdlog/fmt/bundled/xchar.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace http {
namespace server {

void request_handler::handle_request(std::shared_ptr<request> req,
                                     std::shared_ptr<response> rep) {
  spdlog::info("request: {} {} HTTP/{}.{}", req->method, req->request_target,
               req->http_version_major, req->http_version_minor);
  // Decode url to path.
  std::string request_path;
  if (!url_decode(req->request_target, request_path)) {
    response::build_default_response(rep, response::bad_request);
    return;
  }

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/' ||
      request_path.find("..") != std::string::npos) {
    response::build_default_response(rep, response::bad_request);
    return;
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/') {
    request_path += "index.html";
  }

  // Determine the file extension.
  std::size_t last_slash_pos = request_path.find_last_of("/");
  std::size_t last_dot_pos = request_path.find_last_of(".");
  std::string_view extension;
  std::string_view final_path = request_path;
  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
    extension = final_path.substr(last_dot_pos + 1);
  }
  if (request_path[0] == '/') {
    // convert to relative path
    final_path = final_path.substr(1, request_path.size() - 1);
  }

  // Open the file to send back.
  fs::path full_path = m_static_dir / fs::u8path(final_path);
  std::ifstream is(full_path, std::ios::in | std::ios::binary);
  if (!is) {
    response::build_default_response(rep, response::not_found);
    return;
  }

  // Fill out the reply to be sent to the client.
  // TODO: 此处我们发送文件是先将文件全部读取到内存中，然后发送给后端，这样感觉有点整蛊（万一我们要发送很大的文件怎么办？）
  // TODO: 此处需要实现边读取文件，边发送请求，要不然内存占用太高了
  // 提供一个回调来加载数据？
  // we should implement a new streaming response
  rep->status = response::ok;
  std::array<char, 512> buf;
  while (!is.eof()) {
    is.read(buf.data(), buf.size());
    rep->content.append(buf.data(), is.gcount());
  }
  rep->headers["Content-Type"] =
      mime_types::extension_to_type(std::string(extension));
}

// 这个函数需要重写一下，最好使用 antlr 进行解析，需要在 header
// 解析完成后就搞完，目前还是最简单的形式
bool request_handler::url_decode(std::string_view in, std::string &out) {
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        uint8_t b0 = in[i + 1];
        uint8_t b1 = in[i + 2];
        if (std::isxdigit(b0) && std::isxdigit(b1)) {
          uint8_t ch = (b0 << 4) | b1;
          out += static_cast<char>(value);
          i += 2;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }
  return true;
}

} // namespace server
} // namespace http
