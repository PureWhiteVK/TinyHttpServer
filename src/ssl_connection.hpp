#include "base_connection.hpp"
#include <asio.hpp>
#include <asio/ssl.hpp>

namespace http {
namespace server {

class ssl_connection : public base_connection {
public:
  using ssl_stream = asio::ssl::stream<asio::ip::tcp::socket>;
  static std::shared_ptr<ssl_connection>
  create(ssl_stream stream, std::shared_ptr<connection_manager> manager,
         std::shared_ptr<request_handler> handler) {
    return std::shared_ptr<ssl_connection>(
        new ssl_connection(std::move(stream), manager, handler));
  }
  virtual void start();
  virtual void stop();
  virtual void shutdown();

protected:
  explicit ssl_connection(ssl_stream stream,
                          std::shared_ptr<connection_manager> manager,
                          std::shared_ptr<request_handler> handler);

  virtual void do_read();
  virtual void do_write();

  void do_handshake();
  void on_handshaked(std::error_code err);

protected:
  ssl_stream m_stream;
};
} // namespace server
} // namespace http