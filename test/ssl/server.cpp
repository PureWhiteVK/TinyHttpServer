//
// server.cpp
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
#include <functional>
#include <iostream>
#include <spdlog/spdlog.h>

using asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
public:
  session(asio::ssl::stream<tcp::socket> socket) : socket_(std::move(socket)) {}

  void start() { do_handshake(); }

private:
  void do_handshake() {
    spdlog::info("perform ssl handshake");
    auto self(shared_from_this());
    socket_.async_handshake(asio::ssl::stream_base::server,
                            [this, self](const std::error_code &error) {
                              if (!error) {
                                do_read();
                              } else {
                                spdlog::error("{}: {}", error.category().name(),
                                              error.message());
                              }
                            });
  }

  void do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(
        asio::buffer(data_),
        [this, self](const std::error_code &error, std::size_t length) {
          if (!error) {
            spdlog::info("read request");
            do_write(length);
          } else if (error == asio::ssl::error::stream_truncated) {
            spdlog::info("connection closed");
          } else {
            spdlog::error("{}: {} (error_code: {})", error.category().name(),
                          error.message(), error.value());
          }
        });
  }

  void do_write(std::size_t length) {
    spdlog::info("write response");
    auto self(shared_from_this());
    asio::async_write(
        socket_, asio::buffer(data_, length),
        [this, self](const asio::error_code &error, std::size_t /*length*/) {
          if (!error) {
            do_read();
          } else {
            spdlog::error("{}: {} (error_code: {})", error.category().name(),
                          error.message(), error.value());
          }
        });
  }

  asio::ssl::stream<tcp::socket> socket_;
  char data_[1024];
};

class server {
public:
  server(asio::io_context &io_context, unsigned short port)
      : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        context_(asio::ssl::context::sslv23) {
    spdlog::info("start server at port: {}", port);
    context_.set_options(asio::ssl::context::default_workarounds |
                         asio::ssl::context::no_sslv2 |
                         asio::ssl::context::single_dh_use);
    context_.set_password_callback(std::bind(&server::get_password, this));
    spdlog::info("load certificate chain file");
    context_.use_certificate_chain_file("server.pem");
    spdlog::info("load private key file");
    context_.use_private_key_file("server.pem", asio::ssl::context::pem);
    spdlog::info("load tmp dh file");
    context_.use_tmp_dh_file("dh4096.pem");

    do_accept();
  }

private:
  std::string get_password() const { return "test"; }

  void do_accept() {
    acceptor_.async_accept(
        [this](const std::error_code &error, tcp::socket socket) {
          spdlog::info("connection established, remote endpoint: {}.{}",
                       socket.remote_endpoint().address().to_string(),
                       socket.remote_endpoint().port());
          if (!error) {
            std::make_shared<session>(
                asio::ssl::stream<tcp::socket>(std::move(socket), context_))
                ->start();
          } else {
            spdlog::error("{}: {}", error.category().name(), error.message());
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  asio::ssl::context context_;
};

int main(int argc, char *argv[]) {
  try {
    if (argc != 2) {
      spdlog::error("Usage: server <port>");
      return 1;
    }

    asio::io_context io_context;

    using namespace std; // For atoi.
    server s(io_context, atoi(argv[1]));

    io_context.run();
  } catch (std::exception &e) {
    spdlog::error("exception: {}", e.what());
  }

  return 0;
}
