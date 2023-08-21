#include <spdlog/spdlog.h>
#include <sstream>
#include <array>

uint16_t checksum(uint8_t *data, size_t size, uint32_t source_ip,
                  uint32_t target_ip) {
  uint32_t res{0};
  // fake header
  res += (source_ip >> 16);
  res += (source_ip & 0xffff);
  res += (target_ip >> 16);
  res += (target_ip & 0xffff);
  res += 17;
  res += size;
  // actual udp packet
  int i = 0;
  while (i < size) {
    uint16_t byte0 = data[i];
    uint16_t byte1 = i + 1 < size ? data[i + 1] : 0;
    res += (byte0 << 8 | byte1);
    i += 2;
  }
  // 最终结果取反
  return ~((res >> 16) + (res & 0xffff));
}

uint32_t parse_ip(std::string ip) {
  std::stringstream s;
  s.str(ip);
  uint32_t res{};
  std::string v;
  for (int i = 3; i > -1; i--) {
    if (!std::getline(s, v, '.')) {
      throw std::runtime_error(fmt::format("invalid ip: {}", ip));
    }
    uint32_t data = std::stoi(v);
    spdlog::info("{:#08b}", static_cast<uint8_t>(data));
    res |= (data << (i * 8));
  }
  return res;
}

int main() {
  uint32_t source_ip{parse_ip("127.0.0.1")};
  uint32_t target_ip{parse_ip("127.0.0.1")};
  std::array<uint8_t, 33> udp_datagram{
      0x00, 0x0d, 0xc7, 0xd5, 0x00, 0x21, 0x00, 0x00, 0x46, 0x72, 0x69,
      0x20, 0x41, 0x75, 0x67, 0x20, 0x31, 0x38, 0x20, 0x31, 0x36, 0x3a,
      0x33, 0x39, 0x3a, 0x34, 0x38, 0x20, 0x32, 0x30, 0x32, 0x33, 0x0a,
  };
  size_t len_udp_datagram = udp_datagram.size();
  spdlog::info(
      "checksum: {:#04x}",
      checksum(udp_datagram.data(), len_udp_datagram, source_ip, target_ip));
}