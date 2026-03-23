#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

namespace chatapp {

class UserRegistry;

class ChatServer {
 public:
  explicit ChatServer(uint16_t port);
  ~ChatServer();

  ChatServer(const ChatServer&) = delete;
  ChatServer& operator=(const ChatServer&) = delete;

  void run();
  void stop();

 private:
  uint16_t port_;
  int listen_fd_ = -1;
  std::atomic<bool> stopping_{false};
  std::shared_ptr<UserRegistry> registry_;
};

}  // namespace chatapp

