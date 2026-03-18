#pragma once

#include <cstdint>

#include "framing.h"

namespace google::protobuf {
class Message;
}

namespace chatapp {

// Message type values per docs/protocol_standard.md in the shared protocol repo.
enum class MessageType : uint8_t {
  // client -> server
  REGISTER = 1,
  MESSAGE_GENERAL = 2,
  MESSAGE_DM = 3,
  CHANGE_STATUS = 4,
  LIST_USERS = 5,
  GET_USER_INFO = 6,
  QUIT = 7,

  // server -> client
  SERVER_RESPONSE = 10,
  ALL_USERS = 11,
  FOR_DM = 12,
  BROADCAST_MESSAGES = 13,
  GET_USER_INFO_RESPONSE = 14,
};

void send_proto(int fd, MessageType type, const google::protobuf::Message& msg);

}  // namespace chatapp

