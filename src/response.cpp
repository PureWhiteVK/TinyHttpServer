#include "response.hpp"
#include <string>

namespace http {
namespace server {

std::string_view get_status_line(response::status_type status) {
  static std::unordered_map<response::status_type, std::string_view> mappings{
      {response::ok, "HTTP/1.1 200 OK\r\n"},
      {response::created, "HTTP/1.1 201 Created\r\n"},
      {response::accepted, "HTTP/1.1 202 Accepted\r\n"},
      {response::no_content, "HTTP/1.1 204 No Content\r\n"},
      {response::multiple_choices, "HTTP/1.1 300 Multiple Choices\r\n"},
      {response::moved_permanently, "HTTP/1.1 301 Moved Permanently\r\n"},
      {response::moved_temporarily, "HTTP/1.1 302 Moved Temporarily\r\n"},
      {response::not_modified, "HTTP/1.1 304 Not Modified\r\n"},
      {response::bad_request, "HTTP/1.1 400 Bad Request\r\n"},
      {response::unauthorized, "HTTP/1.1 401 Unauthorized\r\n"},
      {response::forbidden, "HTTP/1.1 403 Forbidden\r\n"},
      {response::not_found, "HTTP/1.1 404 Not Found\r\n"},
      {response::internal_server_error,
       "HTTP/1.1 500 Internal Server Error\r\n"},
      {response::not_implemented, "HTTP/1.1 501 Not Implemented\r\n"},
      {response::bad_gateway, "HTTP/1.1 502 Bad Gateway\r\n"},
      {response::service_unavailable, "HTTP/1.1 503 Service Unavailable\r\n"},
  };
  if (mappings.find(status) == mappings.end()) {
    return mappings[response::internal_server_error];
  }
  return mappings[status];
}

std::vector<asio::const_buffer> response::to_buffers() {
  static std::string_view COLON_SP = ": ";
  static std::string_view CRLF = "\r\n";
  std::vector<asio::const_buffer> buffers;
  buffers.emplace_back(asio::buffer(get_status_line(status)));
  for (const auto &[name, value] : headers) {
    buffers.emplace_back(asio::buffer(name));
    buffers.emplace_back(asio::buffer(COLON_SP));
    buffers.emplace_back(asio::buffer(value));
    buffers.emplace_back(asio::buffer(CRLF));
  }
  buffers.emplace_back(asio::buffer(CRLF));
  buffers.emplace_back(asio::buffer(content));
  return buffers;
}

std::string_view get_default_response_content(response::status_type status) {
  static std::unordered_map<response::status_type, std::string_view> mappings{
      {response::ok, "<html>"
                     "<head><title>Created</title></head>"
                     "<body><h1>201 Created</h1></body>"
                     "</html>"},
      {response::created, "<html>"
                          "<head><title>Created</title></head>"
                          "<body><h1>201 Created</h1></body>"
                          "</html>"},
      {response::accepted, "<html>"
                           "<head><title>Accepted</title></head>"
                           "<body><h1>202 Accepted</h1></body>"
                           "</html>"},
      {response::no_content, "<html>"
                             "<head><title>No Content</title></head>"
                             "<body><h1>204 Content</h1></body>"
                             "</html>"},
      {response::multiple_choices,
       "<html>"
       "<head><title>Multiple Choices</title></head>"
       "<body><h1>300 Multiple Choices</h1></body>"
       "</html>"},
      {response::moved_permanently,
       "<html>"
       "<head><title>Moved Permanently</title></head>"
       "<body><h1>301 Moved Permanently</h1></body>"
       "</html>"},
      {response::moved_temporarily,
       "<html>"
       "<head><title>Moved Temporarily</title></head>"
       "<body><h1>302 Moved Temporarily</h1></body>"
       "</html>"},
      {response::not_modified, "<html>"
                               "<head><title>Not Modified</title></head>"
                               "<body><h1>304 Not Modified</h1></body>"
                               "</html>"},
      {response::bad_request, "<html>"
                              "<head><title>Bad Request</title></head>"
                              "<body><h1>400 Bad Request</h1></body>"
                              "</html>"},
      {response::unauthorized, "<html>"
                               "<head><title>Unauthorized</title></head>"
                               "<body><h1>401 Unauthorized</h1></body>"
                               "</html>"},
      {response::forbidden, "<html>"
                            "<head><title>Forbidden</title></head>"
                            "<body><h1>403 Forbidden</h1></body>"
                            "</html>"},
      {response::not_found, "<html>"
                            "<head><title>Not Found</title></head>"
                            "<body><h1>404 Not Found</h1></body>"
                            "</html>"},
      {response::internal_server_error,
       "<html>"
       "<head><title>Internal Server Error</title></head>"
       "<body><h1>500 Internal Server Error</h1></body>"
       "</html>"},
      {response::not_implemented, "<html>"
                                  "<head><title>Not Implemented</title></head>"
                                  "<body><h1>501 Not Implemented</h1></body>"
                                  "</html>"},
      {response::bad_gateway, "<html>"
                              "<head><title>Bad Gateway</title></head>"
                              "<body><h1>502 Bad Gateway</h1></body>"
                              "</html>"},
      {response::service_unavailable,
       "<html>"
       "<head><title>Service Unavailable</title></head>"
       "<body><h1>503 Service Unavailable</h1></body>"
       "</html>"}};

  if (mappings.find(status) == mappings.end()) {
    return mappings[response::internal_server_error];
  }
  return mappings[status];
}

void response::build_default_response(std::shared_ptr<response> rep,
                                      status_type status, bool keep_alive) {
  rep->clear();
  rep->status = status;
  rep->content = get_default_response_content(status);
  rep->headers["Content-Type"] = "text/html";
}

} // namespace server
} // namespace http
