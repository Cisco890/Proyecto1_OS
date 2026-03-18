#include <cstdlib>
#include <iostream>

#include "server/chat_server.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Uso: " << argv[0] << " <puerto>\n";
    return 2;
  }
  const int port_i = std::atoi(argv[1]);
  if (port_i <= 0 || port_i > 65535) {
    std::cerr << "Puerto inválido.\n";
    return 2;
  }

  try {
    chatapp::ChatServer s(static_cast<uint16_t>(port_i));
    s.run();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
}

