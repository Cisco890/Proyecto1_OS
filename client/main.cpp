#include <cstdlib>
#include <iostream>

#include "client/client_app.h"

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Uso: " << argv[0] << " <username> <IP_servidor> <puerto_servidor>\n";
    return 2;
  }

  const std::string username = argv[1];
  const std::string ip = argv[2];
  const int port_i = std::atoi(argv[3]);
  if (port_i <= 0 || port_i > 65535) {
    std::cerr << "Puerto inválido.\n";
    return 2;
  }

  try {
    chatapp::ClientApp app(username, ip, static_cast<uint16_t>(port_i));
    app.run();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
}

