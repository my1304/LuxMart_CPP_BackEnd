#include <boost/asio.hpp>
#include <optional>
#include <nlohmann/json.hpp>
#include "security/jwtService.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

json handleLoginRequest(const std::vector<std::string>& user_data) {
    JwtService jwtService;
    std::string accessToken = jwtService.generateAccessToken(user_data[3]);  // email
    std::string refreshToken = jwtService.generateRefreshToken(user_data[3]); // email

    // Формирование JSON-ответа
    json response_body = {
        {"status", "success"},
        {"message", "Login successful"},
        {"access_token", accessToken},
        {"refresh_token", refreshToken},
        {"user", {
            {"username", user_data[0]},
            {"email", user_data[3]},
            {"first_name", user_data[1]},
            {"last_name", user_data[2]},
            {"roles", user_data[4]}
        }}
    };

    return response_body;
}

std::optional<json> validateJWT(const std::string& token) {
    JwtService jwtService;

    if (jwtService.validateToken(token)) {
        return std::nullopt;  // Возвращаем пустой объект, если токен валиден
    }
    else {
        // Возвращаем JSON-ответ при ошибке валидации токена
        json error_response = {
            {"status", "error"},
            {"message", "Unauthorized"}
        };
        return error_response;
    }
}