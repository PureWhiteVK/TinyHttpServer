#pragma once

#include <filesystem>
#include <spdlog/spdlog.h>
#include <string>

namespace http {
namespace server {

struct response;
struct request;

// we have to implement some kind of register to perform different request

/// The common handler for all incoming requests.
class request_handler {
public:
  request_handler(const request_handler &) = delete;
  request_handler &operator=(const request_handler &) = delete;

  /// Construct with a directory containing files to be served.
  explicit request_handler(const std::filesystem::path& doc_root)
      : m_static_dir(doc_root) {
    spdlog::info("document root: {}", doc_root.u8string());
  }

  /// Handle a request and produce a reply.
  void handle_request(std::shared_ptr<request> req,
                      std::shared_ptr<response> rep);

private:
  /// The directory containing the files to be served.
  std::filesystem::path m_static_dir;

  /// Perform URL-decoding on a string. Returns false if the encoding was
  /// invalid.
  static bool url_decode(std::string_view in, std::string &out);
};

} // namespace server
} // namespace http
