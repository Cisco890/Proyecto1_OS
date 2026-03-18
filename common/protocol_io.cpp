#include "protocol_io.h"

#include <google/protobuf/message.h>
#include <stdexcept>
#include <string>

namespace chatapp {

void send_proto(int fd, MessageType type, const google::protobuf::Message& msg) {
  std::string bytes;
  if (!msg.SerializeToString(&bytes)) {
    throw std::runtime_error("SerializeToString() failed");
  }
  send_framed_message(fd, static_cast<uint8_t>(type), bytes);
}

}  // namespace chatapp

