#include "receiver_loop.h"

#include <iostream>

#include "all_users.pb.h"
#include "broadcast_messages.pb.h"
#include "client/client_app.h"
#include "disconnection_notification.pb.h"
#include "framing.h"
#include "protocol_io.h"
#include "server_broadcast_message.pb.h"
#include "status_change_notification.pb.h"
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
      case MessageType::SERVER_BROADCAST_MESSAGE: {
        chat::ServerBroadcastMessage sbm;
        if (sbm.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::cout << "\n[SERVIDOR] " << sbm.message() << " (" << sbm.timestamp() << ")\n";
        }
        break;
      }
      case MessageType::STATUS_CHANGE_NOTIFICATION: {
        chat::StatusChangeNotification scn;
        if (scn.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::string status_str = (scn.new_status() == chat::ACTIVE) ? "ACTIVO" : 
                                   (scn.new_status() == chat::DO_NOT_DISTURB) ? "OCUPADO" : "INACTIVO";
          std::cout << "\n[NOTIFICACION] " << scn.username() << " ahora esta " << status_str 
                    << " (" << scn.timestamp() << ")\n";
        }
        break;
      }
      case MessageType::DISCONNECTION_NOTIFICATION: {
        chat::DisconnectionNotification dn;
        if (dn.ParseFromArray(fm.payload.data(), static_cast<int>(fm.payload.size()))) {
          std::cout << "\n[NOTIFICACION] " << dn.username() << " se ha desconectado desde " 
                    << dn.ip_address() << " (" << dn.timestamp() << ")\n";
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

