#include "authHandler.h"
#include "security/jwtService.h"
#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

void handleProtectedRequest(tcp::socket& socket, const std::string& token) {
    JwtService jwtService;
    std::cout << "handleProtectedRequest -- username======================" + username << std::endl; //��������� ��� �����

    if (jwtService.validateToken(token)) {
        // JWT �������, ����������� ��������� �������
        std::string username = jwtService.extractUsername(token);
        std::cout << "Authenticated user: " << username << std::endl;
    }
    else {
        // JWT �������������� ��� ���������, ���������� ������
        std::string unauthorized_response = "HTTP/1.1 401 Unauthorized\r\n\r\n";
        boost::asio::write(socket, boost::asio::buffer(unauthorized_response));
    }
}

void authenticateUser(tcp::socket& socket, const std::string& username, const std::string& password) {
    std::cout << "authenticateUser -- username======================" + username << std::endl; //��������� ��� �����

    // ����� �� ����������, ��� ����� � ������ ������������ ���������.
    // ���� ���������, ���������� ������.
    JwtService jwtService;
    std::string accessToken = jwtService.generateAccessToken(username);
    std::string refreshToken = jwtService.generateRefreshToken(username);

    // ��������� HTTP-����� � ��������
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: application/json\r\n";
    response += "\r\n";
    response += "{\"access_token\": \"" + accessToken + "\", \"refresh_token\": \"" + refreshToken + "\"}";

    boost::asio::write(socket, boost::asio::buffer(response));
}