#include "receiver_loop.h"

#include <iostream>

#include "all_users.pb.h"
#include "broadcast_messages.pb.h"
#include "client/client_app.h"
#include "framing.h"
#include "protocol_io.h"
#include "status_mapper.h"
#include "for_dm.pb.h"
#include "get_user_info_response.pb.h"
#include "server_response.pb.h"

namespace chatapp {

void receiver_loop(ClientApp& app) {
  const int fd = app.socket_fd();

  while (true) {
    FramedMessage fm;
    try {
      if (!recv_framed_message(fd, fm)) break;
    } catch (const std::exception& e) {
      std::cerr << "\n[rx] error: " << e.what() << "\n";
      break;
    }

    switch (static_cast<MessageType>(fm.type)) {
      case MessageType::SERVER_RESPONSE: {
        chat::ServerResponse r;
        if (r.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::cerr << "\n[server] (" << r.status_code() << ") " << r.message() << "\n";
        }
        break;
      }
      case MessageType::BROADCAST_MESSAGES: {
        chat::BroadcastDelivery b;
        if (b.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::cout << "\n[" << b.username_origin() << "] " << b.message() << "\n";
        }
        break;
      }
      case MessageType::FOR_DM: {
        chat::ForDm d;
        if (d.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::cout << "\n" << d.message() << "\n";
        }
        break;
      }
      case MessageType::ALL_USERS: {
        chat::AllUsers au;
        if (au.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::cout << "\nUsuarios conectados:\n";
          const int n = au.usernames_size();
          for (int i = 0; i < n; i++) {
            const auto st = (i < au.status_size()) ? au.status(i) : chat::ACTIVE;
            std::cout << "- " << au.usernames(i) << " (" << status_to_pdf_label(st) << ")\n";
          }
        }
        break;
      }
      case MessageType::GET_USER_INFO_RESPONSE: {
        chat::GetUserInfoResponse u;
        if (u.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::cout << "\nInfo usuario:\n";
          std::cout << "- username: " << u.username() << "\n";
          std::cout << "- ip: " << u.ip_address() << "\n";
          std::cout << "- status: " << status_to_pdf_label(u.status()) << "\n";
        }
        break;
      }
      default:
        std::cerr << "\n[rx] tipo desconocido: " << static_cast<int>(fm.type) << "\n";
        break;
    }
  }
}

}  // namespace chatapp

