#include "framing.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace chatapp {
namespace {

void write_all(int fd, const uint8_t* buf, size_t len) {
  size_t off = 0;
  while (off < len) {
    ssize_t n = ::send(fd, buf + off, len - off, 0);
    if (n < 0) {
      if (errno == EINTR) continue;
      throw std::runtime_error(std::string("send() failed: ") + ::strerror(errno));
    }
    if (n == 0) {
      throw std::runtime_error("send() returned 0");
    }
    off += static_cast<size_t>(n);
  }
}

// If we read 0 bytes before any payload, return false (clean close).
// If partial frame and peer closes, throw.
bool read_all_or_eof(int fd, uint8_t* buf, size_t len, bool allow_clean_eof_if_empty) {
  size_t off = 0;
  while (off < len) {
    ssize_t n = ::recv(fd, buf + off, len - off, 0);
    if (n < 0) {
      if (errno == EINTR) continue;
      throw std::runtime_error(std::string("recv() failed: ") + ::strerror(errno));
    }
    if (n == 0) {
      if (allow_clean_eof_if_empty && off == 0) return false;
      throw std::runtime_error("peer closed mid-frame");
    }
    off += static_cast<size_t>(n);
  }
  return true;
}

}  // namespace

void send_framed_message(int fd, uint8_t type, const void* data, size_t len) {
  if (len > 0xFFFFFFFFu) {
    throw std::runtime_error("payload too large");
  }

  uint8_t header[5];
  header[0] = type;
  uint32_t be_len = htonl(static_cast<uint32_t>(len));
  ::memcpy(header + 1, &be_len, sizeof(be_len));

  write_all(fd, header, sizeof(header));
  if (len > 0) {
    write_all(fd, static_cast<const uint8_t*>(data), len);
  }
}

void send_framed_message(int fd, uint8_t type, const std::string& bytes) {
  send_framed_message(fd, type, bytes.data(), bytes.size());
}

void send_framed_message(int fd, uint8_t type, const std::vector<uint8_t>& bytes) {
  send_framed_message(fd, type, bytes.data(), bytes.size());
}

bool recv_framed_message(int fd, FramedMessage& out) {
  uint8_t header[5];
  if (!read_all_or_eof(fd, header, sizeof(header), /*allow_clean_eof_if_empty=*/true)) {
    return false;
  }

  out.type = header[0];
  uint32_t be_len = 0;
  ::memcpy(&be_len, header + 1, sizeof(be_len));
  uint32_t len = ntohl(be_len);

  out.payload.assign(len, 0);
  if (len > 0) {
    (void)read_all_or_eof(fd, out.payload.data(), len, /*allow_clean_eof_if_empty=*/false);
  }
  return true;
}

}  // namespace chatapp

