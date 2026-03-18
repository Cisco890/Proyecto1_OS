#pragma once

#include <optional>
#include <string>

#include "common.pb.h"

namespace chatapp {

std::optional<chat::StatusEnum> parse_status_token(const std::string& token);
std::string status_to_pdf_label(chat::StatusEnum st);

}  // namespace chatapp

