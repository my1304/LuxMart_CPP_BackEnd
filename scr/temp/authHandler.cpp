#include "authHandler.h"
#include "security/jwtService.h"
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

void handleProtectedRequest(tcp::socket& socket, const std::string& token) {
    JwtService jwtService;
    std::cout << "handleProtectedRequest -- username======================" + username << std::endl; //сообще€ие дл€ теста

    if (jwtService.validateToken(token)) {
        // JWT валиден, продолжайте обработку запроса
        std::string username = jwtService.extractUsername(token);
        std::cout << "Authenticated user: " << username << std::endl;
    }
    else {
        // JWT недействителен или просрочен, возвращаем ошибку
        std::string unauthorized_response = "HTTP/1.1 401 Unauthorized\r\n\r\n";
        boost::asio::write(socket, boost::asio::buffer(unauthorized_response));
    }
}

void authenticateUser(tcp::socket& socket, const std::string& username, const std::string& password) {
    std::cout << "authenticateUser -- username======================" + username << std::endl; //сообще€ие дл€ теста

    // «десь вы провер€ете, что логин и пароль пользовател€ корректны.
    // ≈сли корректны, генерируем токены.
    JwtService jwtService;
    std::string accessToken = jwtService.generateAccessToken(username);
    std::string refreshToken = jwtService.generateRefreshToken(username);

    // ‘ормируем HTTP-ответ с токенами
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: application/json\r\n";
    response += "\r\n";
    response += "{\"access_token\": \"" + accessToken + "\", \"refresh_token\": \"" + refreshToken + "\"}";

    boost::asio::write(socket, boost::asio::buffer(response));
}