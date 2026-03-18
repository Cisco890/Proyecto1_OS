#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace chatapp {

struct FramedMessage {
  uint8_t type = 0;
  std::vector<uint8_t> payload;
};

// Returns true on success; false if peer closed cleanly before any bytes were read.
// Throws std::runtime_error on protocol/IO errors.
bool recv_framed_message(int fd, FramedMessage& out);

// Throws std::runtime_error on IO errors.
void send_framed_message(int fd, uint8_t type, const void* data, size_t len);
void send_framed_message(int fd, uint8_t type, const std::string& bytes);
void send_framed_message(int fd, uint8_t type, const std::vector<uint8_t>& bytes);

}  // namespace chatapp

