#include "client_app.h"

#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

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
#include "socket_utils.h"

#include "client/input_loop.h"
#include "client/receiver_loop.h"

namespace chatapp {

ClientApp::ClientApp(std::string username, std::string server_ip, uint16_t server_port)
    : username_(std::move(username)), server_ip_(std::move(server_ip)), server_port_(server_port) {}

ClientApp::~ClientApp() {
  stop();
}

void ClientApp::stop() {
  stopping_.store(true);
  if (fd_ >= 0) {
    ::shutdown(fd_, SHUT_RDWR);
    ::close(fd_);
    fd_ = -1;
  }
  if (rx_thread_.joinable()) rx_thread_.join();
}

void ClientApp::connect_and_register() {
  fd_ = connect_to_server(server_ip_, server_port_);
  my_ip_ = get_local_ip_for_socket(fd_);

  chat::Register r;
  r.set_username(username_);
  r.set_ip(my_ip_);
  {
    std::lock_guard<std::mutex> lk(send_mu_);
    send_proto(fd_, MessageType::REGISTER, r);
  }
}

void ClientApp::run() {
  connect_and_register();

  rx_thread_ = std::thread([this]() { receiver_loop(*this); });
  input_loop(*this);

  stop();
}

void ClientApp::send_broadcast(const std::string& text) {
  chat::MessageGeneral mg;
  mg.set_message(text);
  mg.set_status(current_status());
  mg.set_username_origin(username_);
  mg.set_ip(my_ip_);
  std::lock_guard<std::mutex> lk(send_mu_);
  send_proto(fd_, MessageType::MESSAGE_GENERAL, mg);
}

void ClientApp::send_dm(const std::string& to_user, const std::string& text) {
  chat::MessageDM dm;
  dm.set_message(text);
  dm.set_status(current_status());
  dm.set_username_des(to_user);
  dm.set_ip(my_ip_);
  std::lock_guard<std::mutex> lk(send_mu_);
  send_proto(fd_, MessageType::MESSAGE_DM, dm);
}

void ClientApp::request_list_users() {
  chat::ListUsers req;
  req.set_username(username_);
  req.set_ip(my_ip_);
  std::lock_guard<std::mutex> lk(send_mu_);
  send_proto(fd_, MessageType::LIST_USERS, req);
}

void ClientApp::request_user_info(const std::string& user) {
  chat::GetUserInfo req;
  req.set_username_des(user);
  req.set_username(username_);
  req.set_ip(my_ip_);
  std::lock_guard<std::mutex> lk(send_mu_);
  send_proto(fd_, MessageType::GET_USER_INFO, req);
}

void ClientApp::change_status(chat::StatusEnum st) {
  status_.store(st);
  chat::ChangeStatus cs;
  cs.set_status(st);
  cs.set_username(username_);
  cs.set_ip(my_ip_);
  std::lock_guard<std::mutex> lk(send_mu_);
  send_proto(fd_, MessageType::CHANGE_STATUS, cs);
}

void ClientApp::quit() {
  chat::Quit q;
  q.set_quit(true);
  q.set_ip(my_ip_);
  std::lock_guard<std::mutex> lk(send_mu_);
  send_proto(fd_, MessageType::QUIT, q);
  stopping_.store(true);
}

}  // namespace chatapp

