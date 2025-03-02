#include "security/webConfig.h"
#include <boost/asio.hpp>
#include <set>
#include <string>
#include <iostream>

using boost::asio::ip::tcp;

// Реализация метода для обработки CORS
bool CorsConfig::handleCors(tcp::socket& socket, const std::string& origin, const std::string& method, const std::string& headers) {
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Access-Control-Allow-Origin: " + origin + "\r\n";
//	response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
	response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n";
	response += "Access-Control-Allow-Headers: Authorization, Content-Type\r\n"; 
	response += "Access-Control-Allow-Credentials: true\r\n"; 
	response += "Access-Control-Max-Age: 86400\r\n"; // Cache preflight for 1 day
	response += "Connection: close\r\n\r\n";
	response += "\r\n";

	boost::asio::write(socket, boost::asio::buffer(response));
	return true;
}

// Реализация функции для проверки авторизации запроса и CORS
bool authorizeRequestWithCors(tcp::socket& socket, const std::string& session_id, const std::string& origin, const std::string& method, const std::string& headers) {
	// Проверка сессии
	if (session_id != "valid_session_token") {
		std::string error_response = "HTTP/1.1 403 Forbidden\r\n\r\nInvalid session\n";
		boost::asio::write(socket, boost::asio::buffer(error_response));
		return false;
	}

	// Проверяем CORS
	CorsConfig cors;
	return cors.handleCors(socket, origin, method, headers);
}