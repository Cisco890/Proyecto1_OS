#include "input_loop.h"

#include <iostream>
#include <sstream>
#include <string>

#include "client/client_app.h"
#include "status_mapper.h"

namespace chatapp {
namespace {

void print_help() {
  std::cout
      << "Comandos:\n"
      << "  /help\n"
      << "  /list\n"
      << "  /info <usuario>\n"
      << "  /status <ACTIVO|OCUPADO|INACTIVO>\n"
      << "  /dm <usuario> <mensaje>\n"
      << "  /all <mensaje>\n"
      << "  /quit\n"
      << "Si escribes texto sin '/', se enviará al chat general.\n";
}

}  // namespace

void input_loop(ClientApp& app) {
  print_help();

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.empty()) continue;
    
    // Marcar actividad
    app.mark_activity();

    if (line[0] != '/') {
      app.send_broadcast(line);
      continue;
    }

    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "/help") {
      print_help();
    } else if (cmd == "/list") {
      app.request_list_users();
    } else if (cmd == "/info") {
      std::string user;
      iss >> user;
      if (user.empty()) {
        std::cerr << "Uso: /info <usuario>\n";
        continue;
      }
      app.request_user_info(user);
    } else if (cmd == "/status") {
      std::string st;
      iss >> st;
      if (st.empty()) {
        std::cerr << "Uso: /status <ACTIVO|OCUPADO|INACTIVO>\n";
        continue;
      }
      auto parsed = parse_status_token(st);
      if (!parsed) {
        std::cerr << "Status inválido.\n";
        continue;
      }
      app.change_status(*parsed);
    } else if (cmd == "/dm") {
      std::string to;
      iss >> to;
      std::string msg;
      std::getline(iss, msg);
      if (!msg.empty() && msg[0] == ' ') msg.erase(0, 1);
      if (to.empty() || msg.empty()) {
        std::cerr << "Uso: /dm <usuario> <mensaje>\n";
        continue;
      }
      app.send_dm(to, msg);
    } else if (cmd == "/all") {
      std::string msg;
      std::getline(iss, msg);
      if (!msg.empty() && msg[0] == ' ') msg.erase(0, 1);
      if (msg.empty()) {
        std::cerr << "Uso: /all <mensaje>\n";
        continue;
      }
      app.send_broadcast(msg);
    } else if (cmd == "/quit") {
      app.quit();
      break;
    } else {
      std::cerr << "Comando desconocido. Usa /help\n";
    }
  }
}

}  // namespace chatapp

