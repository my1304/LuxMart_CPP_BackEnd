#include "security/jwtService.h"
#include "security/jwtConfig.h"
#include <jwt-cpp/jwt.h>
#include <chrono>

std::string JwtService::generateRefreshToken(const std::string& username) {
    auto token = jwt::create()
        .set_issuer("auth_service")
        .set_type("JWT")
        .set_subject(username)
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(REFRESH_TOKEN_EXPIRATION))
        .sign(jwt::algorithm::hs256{ REFRESH_TOKEN_SECRET });

    return token;
}
std::string JwtService::generateAccessToken(const std::string& username) {
    auto token = jwt::create()
        .set_issuer("auth_service")
        .set_type("JWT")
        .set_subject(username)
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(ACCESS_TOKEN_EXPIRATION))
        .sign(jwt::algorithm::hs256{ ACCESS_TOKEN_SECRET });

    return token;
}

bool JwtService::validateToken(const std::string& token) {
    try {
        auto decoded_token = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{ ACCESS_TOKEN_SECRET })
            .with_issuer("auth_service");
        verifier.verify(decoded_token);
        return true;
    }
    catch (const std::exception& e) {
        // Ошибка при проверке токена
        std::cout << "Error check Token--- : " << std::endl; // сообщение для теста
        return false;
    }
}

std::string JwtService::extractUsername(const std::string& token) {
    auto decoded_token = jwt::decode(token);
    return decoded_token.get_subject();
}