#pragma once

#include <string>

#include "server/user_registry.h"

namespace chatapp {

class SessionHandler {
 public:
  static void handle_client(int fd, const std::string& peer_ip, UserRegistry& reg);
};

}  // namespace chatapp

