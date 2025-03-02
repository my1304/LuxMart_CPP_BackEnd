#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

json handleLoginRequest(const std::vector<std::string>& user_data);
std::optional<json> validateJWT(const std::string& token);

#endif // AUTH_CONTROLLER_H