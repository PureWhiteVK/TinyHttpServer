//
// timer.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <system_error>

void print(const asio::error_code & /*e*/) { spdlog::info("Hello, world!"); }

class Foo {
public:
  void dummy_func() {
    spdlog::info("this is a stupid member function of class Foo.");
  }
};

int main() {
  asio::io_context io;
  auto wait_time = std::chrono::seconds(1);
  asio::steady_timer t(io, wait_time);
  t.async_wait([&](const std::error_code &e) {
    spdlog::info(
        "this will print after {}s!",
        std::chrono::duration_cast<std::chrono::seconds>(wait_time).count());
  });
  Foo foo;
  t.async_wait(std::bind(&Foo::dummy_func, &foo));
  t.async_wait(print);
  spdlog::info("this will print immediately!");
  io.run();
  
  return 0;
}