#pragma once

#include <cstdint>
#include <string>

namespace chatapp {

int create_listen_socket(uint16_t port, int backlog = 16);
int accept_client(int listen_fd, std::string& out_ip);
int connect_to_server(const std::string& ip, uint16_t port);

std::string get_local_ip_for_socket(int fd);

}  // namespace chatapp

