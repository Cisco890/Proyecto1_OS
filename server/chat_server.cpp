#include "chat_server.h"

#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

#include "socket_utils.h"
#include "server/session_handler.h"
#include "server/user_registry.h"

namespace chatapp {
namespace {

std::atomic<bool>* g_stop_flag = nullptr;

void on_sigint(int) {
  if (g_stop_flag) g_stop_flag->store(true);
}

}  // namespace

ChatServer::ChatServer(uint16_t port) : port_(port) {}

ChatServer::~ChatServer() {
  stop();
}

void ChatServer::stop() {
  stopping_.store(true);
  if (listen_fd_ >= 0) {
    ::close(listen_fd_);
    listen_fd_ = -1;
  }
}

void ChatServer::run() {
  UserRegistry registry;

  listen_fd_ = create_listen_socket(port_);

  g_stop_flag = &stopping_;
  ::signal(SIGINT, on_sigint);

  std::cerr << "[server] escuchando en puerto " << port_ << "\n";

  while (!stopping_.load()) {
    std::string peer_ip;
    int cfd = -1;
    try {
      cfd = accept_client(listen_fd_, peer_ip);
    } catch (const std::exception& e) {
      if (!stopping_.load()) std::cerr << "[server] accept error: " << e.what() << "\n";
      continue;
    }
    if (cfd < 0) continue;

    std::thread([cfd, peer_ip, &registry]() { SessionHandler::handle_client(cfd, peer_ip, registry); })
        .detach();
  }

  std::cerr << "[server] apagando...\n";
  stop();
}

}  // namespace chatapp

