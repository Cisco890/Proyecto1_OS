#include "session_handler.h"

#include <unistd.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "all_users.pb.h"
#include "broadcast_messages.pb.h"
#include "change_status.pb.h"
#include "for_dm.pb.h"
#include "framing.h"
#include "get_user_info.pb.h"
#include "get_user_info_response.pb.h"
#include "list_users.pb.h"
#include "message_dm.pb.h"
#include "message_general.pb.h"
#include "protocol_io.h"
#include "quit.pb.h"
#include "register.pb.h"
#include "server_response.pb.h"

namespace chatapp {
namespace {

chat::ServerResponse make_response(int code, const std::string& msg, bool ok) {
  chat::ServerResponse r;
  r.set_status_code(code);
  r.set_message(msg);
  r.set_is_successful(ok);
  return r;
}

void send_response(int fd, int code, const std::string& msg, bool ok) {
  chat::ServerResponse r = make_response(code, msg, ok);
  send_proto(fd, MessageType::SERVER_RESPONSE, r);
}

void broadcast(UserRegistry& reg, const chat::BroadcastDelivery& b) {
  std::vector<std::string> users;
  std::vector<chat::StatusEnum> st;
  reg.snapshot_users(users, st);
  for (const auto& u : users) {
    auto sess = reg.get_by_username(u);
    if (!sess) continue;
    try {
      send_proto(sess->fd, MessageType::BROADCAST_MESSAGES, b);
    } catch (...) {
      // ignore; session cleanup is handled elsewhere on next IO
    }
  }
}

}  // namespace

void SessionHandler::handle_client(int fd, const std::string& peer_ip, UserRegistry& reg) {
  std::string username;

  try {
    // Require first message to be REGISTER.
    FramedMessage fm;
    if (!recv_framed_message(fd, fm)) {
      ::close(fd);
      return;
    }
    if (fm.type != static_cast<uint8_t>(MessageType::REGISTER)) {
      send_response(fd, 400, "Primero debes registrarte (type=1).", false);
      ::close(fd);
      return;
    }

    chat::Register r;
    if (!r.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
      send_response(fd, 400, "Registro inválido (protobuf).", false);
      ::close(fd);
      return;
    }
    username = r.username();
    const std::string ip = r.ip().empty() ? peer_ip : r.ip();

    std::string err;
    if (!reg.try_register_user(username, ip, fd, err)) {
      send_response(fd, 409, "Registro rechazado: " + err, false);
      ::close(fd);
      return;
    }
    send_response(fd, 200, "Registrado como '" + username + "'", true);

    // Main loop.
    while (true) {
      FramedMessage msg;
      if (!recv_framed_message(fd, msg)) break;
      reg.touch(fd);

      const auto me = reg.get_by_fd(fd);
      if (!me) {
        send_response(fd, 401, "Sesión inválida (no registrada).", false);
        break;
      }

      switch (static_cast<MessageType>(msg.type)) {
        case MessageType::MESSAGE_GENERAL: {
          chat::MessageGeneral mg;
          if (!mg.ParseFromArray(msg.payload.data(), static_cast<int>(msg.payload.size()))) {
            send_response(fd, 400, "message_general inválido.", false);
            break;
          }
          chat::BroadcastDelivery b;
          b.set_username_origin(me->username);
          b.set_message(mg.message());
          broadcast(reg, b);
          break;
        }
        case MessageType::MESSAGE_DM: {
          chat::MessageDM dm;
          if (!dm.ParseFromArray(msg.payload.data(), static_cast<int>(msg.payload.size()))) {
            send_response(fd, 400, "message_dm inválido.", false);
            break;
          }
          auto dest = reg.get_by_username(dm.username_des());
          if (!dest) {
            send_response(fd, 404, "Usuario destino no existe.", false);
            break;
          }
          chat::ForDm f;
          f.set_username_des(dm.username_des());
          std::ostringstream oss;
          oss << "[DM de " << me->username << "] " << dm.message();
          f.set_message(oss.str());
          send_proto(dest->fd, MessageType::FOR_DM, f);
          break;
        }
        case MessageType::CHANGE_STATUS: {
          chat::ChangeStatus cs;
          if (!cs.ParseFromArray(msg.payload.data(), static_cast<int>(msg.payload.size()))) {
            send_response(fd, 400, "change_status inválido.", false);
            break;
          }
          if (!reg.set_status(me->username, cs.status())) {
            send_response(fd, 404, "No se pudo actualizar status.", false);
          } else {
            send_response(fd, 200, "Status actualizado.", true);
          }
          break;
        }
        case MessageType::LIST_USERS: {
          chat::AllUsers au;
          std::vector<std::string> users;
          std::vector<chat::StatusEnum> st;
          reg.snapshot_users(users, st);
          for (const auto& u : users) au.add_usernames(u);
          for (auto s : st) au.add_status(s);
          send_proto(fd, MessageType::ALL_USERS, au);
          break;
        }
        case MessageType::GET_USER_INFO: {
          chat::GetUserInfo gi;
          if (!gi.ParseFromArray(msg.payload.data(), static_cast<int>(msg.payload.size()))) {
            send_response(fd, 400, "get_user_info inválido.", false);
            break;
          }
          auto u = reg.get_by_username(gi.username_des());
          if (!u) {
            send_response(fd, 404, "Usuario no existe.", false);
            break;
          }
          chat::GetUserInfoResponse resp;
          resp.set_username(u->username);
          resp.set_ip_address(u->ip);
          resp.set_status(u->status);
          send_proto(fd, MessageType::GET_USER_INFO_RESPONSE, resp);
          break;
        }
        case MessageType::QUIT: {
          send_response(fd, 200, "Bye.", true);
          goto done;
        }
        case MessageType::REGISTER:
          send_response(fd, 400, "Ya estás registrado.", false);
          break;
        default:
          send_response(fd, 400, "Tipo de mensaje no soportado.", false);
          break;
      }
    }
  } catch (const std::exception&) {
    // fallthrough to cleanup
  }

done:
  reg.unregister_by_fd(fd);
  ::close(fd);
}

}  // namespace chatapp

