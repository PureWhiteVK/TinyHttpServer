#include <asio.hpp>
#include <spdlog/spdlog.h>

class Printer {
public:
  Printer(asio::io_context &io)
      : m_strand(asio::make_strand(io)), m_timer1(io, asio::chrono::seconds(1)),
        m_timer2(io, asio::chrono::milliseconds(500)) {
    m_timer1.async_wait(
        asio::bind_executor(m_strand, std::bind(&Printer::print1, this)));
    m_timer2.async_wait(
        asio::bind_executor(m_strand, std::bind(&Printer::print2, this)));
  }
  ~Printer() { spdlog::info("final count is {}.", m_count); }

  void print1() {
    if (m_count < 10) {
      spdlog::info("Timer 1: {}", m_count);
      m_count++;
      // 延长过期时间，便于继续执行函数
      m_timer1.expires_at(m_timer1.expiry() + asio::chrono::seconds(1));

      m_timer1.async_wait(
          asio::bind_executor(m_strand, std::bind(&Printer::print1, this)));
    }
  }

  void print2() {
    if (m_count < 10) {
      spdlog::info("Timer 2: {}", m_count);
      m_count++;

      m_timer2.expires_at(m_timer2.expiry() + asio::chrono::milliseconds(500));

      m_timer2.async_wait(
          asio::bind_executor(m_strand, std::bind(&Printer::print2, this)));
    }
  }

private:
  // strand 原本意思为 一股线，这也很形象，所有的函数调用都是严格按照顺序执行的（有一个队列记录顺序）
  // A strand is defined as a strictly sequential invocation of event handlers (i.e. no concurrent invocation). 
  asio::strand<asio::io_context::executor_type> m_strand;
  asio::steady_timer m_timer1;
  asio::steady_timer m_timer2;
  int m_count{};
};

int main() {
  asio::io_context io;
  Printer printer(io);
  asio::thread t([&]() { io.run(); });
  io.run();
  t.join();
}