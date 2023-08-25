//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <spdlog/spdlog.h>

using asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

enum { max_length = 1024 };

class client {
public:
  client(asio::io_context &io_context, asio::ssl::context &context,
         const tcp::resolver::results_type &endpoints)
      : socket_(io_context, context) {
    socket_.set_verify_mode(asio::ssl::verify_peer);
    socket_.set_verify_callback(
        std::bind(&client::verify_certificate, this, _1, _2));

    connect(endpoints);
  }

private:
  bool verify_certificate(bool preverified, asio::ssl::verify_context &ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    spdlog::info("verifying {}", subject_name);
    // preverified = false;
    // spdlog::info("preverified: {}",preverified);
    return preverified;
  }

  void connect(const tcp::resolver::results_type &endpoints) {
    asio::async_connect(socket_.lowest_layer(), endpoints,
                        [this](const std::error_code &error,
                               const tcp::endpoint & /*endpoint*/) {
                          if (!error) {
                            handshake();
                          } else {
                            spdlog::error("{}: {}", error.category().name(),
                                          error.message());
                          }
                        });
  }

  void handshake() {
    socket_.async_handshake(
        asio::ssl::stream_base::client, [this](const std::error_code &error) {
          if (!error) {
            send_request();
          } else {
            spdlog::error("{}: {}", error.category().name(), error.message());
          }
        });
  }

  void send_request() {
    std::cout << "Enter message: ";
    std::cin.getline(request_, max_length);
    size_t request_length = std::strlen(request_);

    asio::async_write(socket_, asio::buffer(request_, request_length),
                      [this](const std::error_code &error, std::size_t length) {
                        if (!error) {
                          receive_response(length);
                        } else {
                          spdlog::error("{}: {}", error.category().name(),
                                        error.message());
                        }
                      });
  }

  void receive_response(std::size_t length) {
    asio::async_read(socket_, asio::buffer(reply_, length),
                     [this](const std::error_code &error, std::size_t length) {
                       if (!error) {
                         std::cout << "Reply: ";
                         std::cout.write(reply_, length);
                         std::cout << "\n";
                       } else {
                         spdlog::error("{}: {}", error.category().name(),
                                       error.message());
                       }
                     });
  }

  asio::ssl::stream<tcp::socket> socket_;
  char request_[max_length];
  char reply_[max_length];
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      std::cerr << "Usage: client <host> <port>\n";
      return 1;
    }

    asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);

    asio::ssl::context ctx(asio::ssl::context::tlsv12);
    spdlog::info("load ssl private key");
    ctx.load_verify_file("ca.pem");

    client c(io_context, ctx, endpoints);

    io_context.run();
  } catch (std::exception &e) {
    spdlog::error("exception: {}", e.what());
  }

  return 0;
}
