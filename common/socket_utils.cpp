#include "socket_utils.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace chatapp {
namespace {

void throw_errno(const char* what) {
  throw std::runtime_error(std::string(what) + ": " + ::strerror(errno));
}

}  // namespace

int create_listen_socket(uint16_t port, int backlog) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) throw_errno("socket()");

  int opt = 1;
  (void)::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    ::close(fd);
    throw_errno("bind()");
  }
  if (::listen(fd, backlog) < 0) {
    ::close(fd);
    throw_errno("listen()");
  }
  return fd;
}

int accept_client(int listen_fd, std::string& out_ip) {
  sockaddr_in client{};
  socklen_t len = sizeof(client);
  int cfd = ::accept(listen_fd, reinterpret_cast<sockaddr*>(&client), &len);
  if (cfd < 0) {
    if (errno == EINTR) return -1;
    throw_errno("accept()");
  }

  char ipbuf[INET_ADDRSTRLEN]{0};
  const char* res = ::inet_ntop(AF_INET, &client.sin_addr, ipbuf, sizeof(ipbuf));
  out_ip = res ? std::string(res) : std::string();
  return cfd;
}

int connect_to_server(const std::string& ip, uint16_t port) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) throw_errno("socket()");

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
    ::close(fd);
    throw std::runtime_error("invalid IPv4 address: " + ip);
  }

  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    ::close(fd);
    throw_errno("connect()");
  }
  return fd;
}

std::string get_local_ip_for_socket(int fd) {
  sockaddr_in local{};
  socklen_t len = sizeof(local);
  if (::getsockname(fd, reinterpret_cast<sockaddr*>(&local), &len) < 0) {
    throw_errno("getsockname()");
  }
  char ipbuf[INET_ADDRSTRLEN]{0};
  const char* res = ::inet_ntop(AF_INET, &local.sin_addr, ipbuf, sizeof(ipbuf));
  return res ? std::string(res) : std::string();
}

}  // namespace chatapp

