#include "chat_server.h"

#include <cerrno>
#include <cstring>
#include <signal.h>
#include <sys/select.h>
#include <unistd.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "protocol_io.h"
#include "server_broadcast_message.pb.h"
#include "socket_utils.h"
#include "server/session_handler.h"
#include "server/user_registry.h"

namespace chatapp {
namespace {

std::atomic<bool>* g_stop_flag = nullptr;

void on_sigint(int) {
  if (g_stop_flag) g_stop_flag->store(true);
}

std::string get_current_timestamp() {
  auto now = std::time(nullptr);
  auto tm = *std::localtime(&now);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

void broadcast_server_message(UserRegistry& reg, const std::string& message) {
  chat::ServerBroadcastMessage sbm;
  sbm.set_message(message);
  sbm.set_timestamp(get_current_timestamp());
  
  std::vector<std::string> users;
  std::vector<chat::StatusEnum> st;
  reg.snapshot_users(users, st);
  for (const auto& u : users) {
    auto sess = reg.get_by_username(u);
    if (!sess) continue;
    try {
      send_proto(sess->fd, MessageType::SERVER_BROADCAST_MESSAGE, sbm);
    } catch (...) {}
  }
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
  registry_ = std::make_shared<UserRegistry>();

  listen_fd_ = create_listen_socket(port_);

  g_stop_flag = &stopping_;
  ::signal(SIGINT, on_sigint);

  std::cerr << "[server] escuchando en puerto " << port_ << "\n";
  std::cerr << "[server] escribe comandos: /msg <mensaje> o /close\n";

  // Thread para entrada de comandos del servidor
  std::thread input_thread([this]() {
    std::string line;
    while (!stopping_.load()) {
      if (!std::getline(std::cin, line)) break;
      if (line.empty()) continue;
      
      if (line == "/close") {
        std::cerr << "[server] cerrando...\n";
        stopping_.store(true);
        break;
      } else if (line.substr(0, 5) == "/msg ") {
        std::string msg = line.substr(5);
        if (!msg.empty()) {
          broadcast_server_message(*registry_, msg);
        }
      } else if (line == "/help") {
        std::cout << "Comandos del servidor:\n";
        std::cout << "  /msg <mensaje> - Enviar mensaje global a todos\n";
        std::cout << "  /close - Apagar servidor\n";
      } else {
        std::cout << "Comando desconocido. Escribe /help\n";
      }
    }
  });

  while (!stopping_.load()) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listen_fd_, &readfds);
    
    struct timeval timeout{};
    timeout.tv_sec = 1;   // 1 segundo de timeout
    timeout.tv_usec = 0;
    
    int select_result = ::select(listen_fd_ + 1, &readfds, nullptr, nullptr, &timeout);
    
    if (select_result < 0) {
      if (errno == EINTR) continue;
      std::cerr << "[server] select error: " << strerror(errno) << "\n";
      continue;
    }
    
    if (select_result == 0) {
      // Timeout, revisar stopping_ y continuar
      continue;
    }
    
    // Hay una conexión disponible
    if (FD_ISSET(listen_fd_, &readfds)) {
      std::string peer_ip;
      int cfd = -1;
      try {
        cfd = accept_client(listen_fd_, peer_ip);
      } catch (const std::exception& e) {
        if (!stopping_.load()) std::cerr << "[server] accept error: " << e.what() << "\n";
        continue;
      }
      if (cfd < 0) continue;

      std::thread([cfd, peer_ip, this]() { SessionHandler::handle_client(cfd, peer_ip, *registry_); })
          .detach();
    }
  }

  std::cerr << "[server] apagando...\n";
  stop();
  
  // Esperar a que el input_thread termine
  if (input_thread.joinable()) {
    input_thread.join();
  }
}



}  // namespace chatapp

