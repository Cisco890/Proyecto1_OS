#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "common.pb.h"

namespace chatapp {

struct UserSession {
  std::string username;
  std::string ip;
  int fd = -1;
  chat::StatusEnum status = chat::ACTIVE;
  std::chrono::steady_clock::time_point last_activity = std::chrono::steady_clock::now();
};

class UserRegistry {
 public:
  bool try_register_user(const std::string& username, const std::string& ip, int fd, std::string& err);
  void unregister_by_fd(int fd);

  std::optional<UserSession> get_by_username(const std::string& username) const;
  std::optional<UserSession> get_by_fd(int fd) const;

  bool set_status(const std::string& username, chat::StatusEnum st);
  void touch(int fd);

  void snapshot_users(std::vector<std::string>& usernames, std::vector<chat::StatusEnum>& statuses) const;

 private:
  mutable std::mutex mu_;
  std::unordered_map<std::string, UserSession> by_user_;
  std::unordered_map<int, std::string> user_by_fd_;
  std::unordered_map<std::string, std::string> user_by_ip_;
};

}  // namespace chatapp

