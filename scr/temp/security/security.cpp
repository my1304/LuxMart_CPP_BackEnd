#include "security/security.h"
#include <boost/asio.hpp>
#include <set>
#include <string>
#include <iostream>
#include <utility>  // для std::pair

using boost::asio::ip::tcp;

// Функция для извлечения имени сервера и порта из origin
std::pair<std::string, std::string> serverNamePort(const std::string& origin) {
	std::string server_name;
	std::string port;

	// Находим начало имени сервера после "http://"
	std::size_t protocol_pos = origin.find("://");
	std::size_t server_start = (protocol_pos == std::string::npos) ? (0) : (protocol_pos + 3);
	std::size_t port_start = origin.find(":", server_start);  // Начало порта

	// Извлечение имени сервера
	if (port_start != std::string::npos) {
		server_name = origin.substr(server_start, port_start - server_start);

		// Извлечение номера порта
		std::size_t port_end = origin.find("/", port_start);
		if (port_end != std::string::npos) {
			port = origin.substr(port_start + 1, port_end - port_start - 1);
		}
		else {
			port = origin.substr(port_start + 1);
		}
		std::cerr << "server_name, port: [" << server_name << "]  -- [" << port << "]" << std::endl;

		return { server_name, port };
	}	
	return { "", "" };// Возвращаем пустую пару, если не удалось извлечь имя сервера и порт
}

bool sendToFrontend(const std::string& response, const std::string& origin) {
	try {
		auto server_info = serverNamePort(origin);  // Получаем пару {server_name, port}
		std::cerr << "sendToFrontend----origin_name [" << server_info.first << "]  -- [" << server_info.second << "]" << std::endl;
		std::cerr << "response: " << response << std::endl;
		boost::asio::io_context io_context;
		// Резолвинг адреса и порта
		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve(server_info.first, server_info.second);

		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);

		boost::asio::write(socket, boost::asio::buffer(response)); // Отправка данных на указанный адрес и порт
		return true;
	}
	catch (std::exception& e) {
		std::cerr << "Ошибка при отправке данных: " << e.what() << std::endl;
		return false;
	}
}

// Реализация метода для обработки CORS
bool CorsConfig::handleCors(tcp::socket& socket, const std::string& origin) {

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: application/json\r\n";
	response += "Access-Control-Allow-Origin: " + origin + "\r\n";
	response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
	response += "Access-Control-Allow-Headers: Authorization, Content-Type\r\n";
	response += "Access-Control-Allow-Credentials: true\r\n";
	response += "Access-Control-Max-Age: 86400\r\n"; // Кэширование preflight запроса на 1 день
	response += "\r\n";

	// Отправляем ответ на указанный сервер и порт
	return sendToFrontend(response, origin);
}



/*
// Реализация функции для проверки авторизации запроса и CORS
bool authorizeRequestWithCors(tcp::socket& socket, const std::string& session_id) {
	// Проверка сессии
	if (session_id != "valid_session_token") {
		std::string error_response = "HTTP/1.1 403 Forbidden\r\n\r\nInvalid session\n";
		boost::asio::write(socket, boost::asio::buffer(error_response));
		return false;
	}

	// Проверяем CORS
	CorsConfig cors;
	return cors.handleCors(socket);
}
*/