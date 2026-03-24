#include "user_registry.h"

#include <unistd.h>
#include <vector>

namespace chatapp {

bool UserRegistry::try_register_user(const std::string& username, const std::string& ip, int fd, std::string& err) {
  std::lock_guard<std::mutex> lk(mu_);
  if (username.empty()) {
    err = "username vacío";
    return false;
  }
  if (by_user_.count(username)) {
    err = "username ya existe";
    return false;
  }
  if (!ip.empty() && user_by_ip_.count(ip)) {
    err = "IP ya registrada";
    return false;
  }
  UserSession s;
  s.username = username;
  s.ip = ip;
  s.fd = fd;
  s.status = chat::ACTIVE;
  s.last_activity = std::chrono::steady_clock::now();

  by_user_[username] = s;
  user_by_fd_[fd] = username;
  if (!ip.empty()) user_by_ip_[ip] = username;
  return true;
}

void UserRegistry::unregister_by_fd(int fd) {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = user_by_fd_.find(fd);
  if (it == user_by_fd_.end()) return;
  const std::string username = it->second;
  user_by_fd_.erase(it);

  auto uit = by_user_.find(username);
  if (uit != by_user_.end()) {
    if (!uit->second.ip.empty()) user_by_ip_.erase(uit->second.ip);
    by_user_.erase(uit);
  }
}

std::optional<UserSession> UserRegistry::get_by_username(const std::string& username) const {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = by_user_.find(username);
  if (it == by_user_.end()) return std::nullopt;
  return it->second;
}

std::optional<UserSession> UserRegistry::get_by_fd(int fd) const {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = user_by_fd_.find(fd);
  if (it == user_by_fd_.end()) return std::nullopt;
  auto uit = by_user_.find(it->second);
  if (uit == by_user_.end()) return std::nullopt;
  return uit->second;
}

bool UserRegistry::set_status(const std::string& username, chat::StatusEnum st) {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = by_user_.find(username);
  if (it == by_user_.end()) return false;
  it->second.status = st;
  it->second.last_activity = std::chrono::steady_clock::now();
  return true;
}

void UserRegistry::touch(int fd) {
  std::lock_guard<std::mutex> lk(mu_);
  auto it = user_by_fd_.find(fd);
  if (it == user_by_fd_.end()) return;
  auto uit = by_user_.find(it->second);
  if (uit == by_user_.end()) return;
  uit->second.last_activity = std::chrono::steady_clock::now();
}

void UserRegistry::snapshot_users(std::vector<std::string>& usernames,
                                 std::vector<chat::StatusEnum>& statuses) const {
  std::lock_guard<std::mutex> lk(mu_);
  usernames.clear();
  statuses.clear();
  usernames.reserve(by_user_.size());
  statuses.reserve(by_user_.size());
  for (const auto& [u, sess] : by_user_) {
    usernames.push_back(u);
    statuses.push_back(sess.status);
  }
}

void UserRegistry::close_all_clients() {
  std::lock_guard<std::mutex> lk(mu_);
  for (const auto& [fd, username] : user_by_fd_) {
    if (fd >= 0) {
      ::close(fd);
    }
  }
}

}  // namespace chatapp

