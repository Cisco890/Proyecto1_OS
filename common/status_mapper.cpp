#include "status_mapper.h"

#include <algorithm>
#include <cctype>

namespace chatapp {
namespace {

std::string upper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
  return s;
}

}  // namespace

std::optional<chat::StatusEnum> parse_status_token(const std::string& token) {
  const std::string t = upper(token);
  if (t == "ACTIVO" || t == "ACTIVE") return chat::ACTIVE;
  if (t == "OCUPADO" || t == "DND" || t == "DO_NOT_DISTURB") return chat::DO_NOT_DISTURB;
  if (t == "INACTIVO" || t == "INVISIBLE") return chat::INVISIBLE;
  return std::nullopt;
}

std::string status_to_pdf_label(chat::StatusEnum st) {
  switch (st) {
    case chat::ACTIVE:
      return "ACTIVO";
    case chat::DO_NOT_DISTURB:
      return "OCUPADO";
    case chat::INVISIBLE:
      return "INACTIVO";
    default:
      return "ACTIVO";
  }
}

}  // namespace chatapp

