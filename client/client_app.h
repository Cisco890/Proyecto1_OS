#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

#include "common.pb.h"

namespace chatapp {

class ClientApp {
 public:
  ClientApp(std::string username, std::string server_ip, uint16_t server_port);
  ~ClientApp();

  void run();

  // Thread-safe send helpers
  void send_broadcast(const std::string& text);
  void send_dm(const std::string& to_user, const std::string& text);
  void request_list_users();
  void request_user_info(const std::string& user);
  void change_status(chat::StatusEnum st);
  void quit();

  const std::string& username() const { return username_; }
  int socket_fd() const { return fd_; }
  chat::StatusEnum current_status() const { return status_.load(); }

  void stop();

 private:
  void connect_and_register();

  std::string username_;
  std::string server_ip_;
  uint16_t server_port_;

  int fd_ = -1;
  std::string my_ip_;

  std::mutex send_mu_;

  std::atomic<bool> stopping_{false};
  std::atomic<chat::StatusEnum> status_{chat::ACTIVE};

  std::thread rx_thread_;
};

}  // namespace chatapp

