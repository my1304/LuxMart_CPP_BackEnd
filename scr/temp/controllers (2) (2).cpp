#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "database/dbOperations.h"  
#include "security/authController.h"
#include "services/functions.h"
#include "security/security.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

CorsConfig corsConfig;

void closeSocket(tcp::socket& socket) {
	boost::system::error_code error;
	socket.shutdown(tcp::socket::shutdown_both, error);
	if (!error) {
		socket.close();
		std::cout << "socket.closed!!!" << std::endl; // сообщение для теста		

	}
}
bool isSocketReady(tcp::socket& socket) {
	try {
		// Проверяем, что сокет открыт
		if (!socket.is_open()) {
			std::cerr << "Socket is not open!" << std::endl;
			return false;
		}

		// Проверяем наличие данных в потоке с использованием метода available()
		boost::system::error_code ec;
		//std::size_t availableData = socket.available(ec);
		if (ec) {
			std::cerr << "Error checking socket availability: " << ec.message() << std::endl;
			return false;
		}		
		/*
		if (availableData == 0) {
			std::cerr << "No data available in the socket buffer." << std::endl;
			return false;
		}
		*/
		return true;		// Сокет готов к чтению
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in isSocketReady: " << e.what() << std::endl;
		return false;
	}
}

// Hash function to get a unique integer value for a string
std::size_t hash_uri(const std::string& uri) {
	return std::hash<std::string>{}(uri);
}

// Function that processes requests depending on URI
void handleRequest(tcp::socket& socket) {
	boost::system::error_code error;
	boost::asio::streambuf request;
	std::cout << "Test message - Enter Access****1111111111111111111111111111111" << std::endl; // сообщение для теста		
	if (isSocketReady(socket)) {
		boost::asio::read_until(socket, request, "\r\n\r\n", error);
		if (error) {
			std::cerr << "---------------Failed to read request: " << error.message() << std::endl;
			closeSocket(socket);
			return;
		}
	}
	std::cout << "Test message - Enter Access****2222222222222222222222222222222" << std::endl; // сообщение для теста
	// Convert the entire request to a string (including headers and body)
	std::istream request_stream(&request);
	std::string method, uri, http_version;
	request_stream >> method >> uri >> http_version;
	std::ostringstream request_body_stream;
	request_body_stream << request_stream.rdbuf();  // Read everything into one buffer
	std::string body = request_body_stream.str();   // Read everything into a string

	std::cout << "Test message - method  :" << method << std::endl; // сообщение для теста	
	std::cout << "Test message - body  -----------start----------------------------------------" << std::endl; // сообщение для теста	
	std::cout << "Test message - body  :" << body << std::endl; // сообщение для теста	
	std::cout << "Test message - body  --------end-------------------------------------------" << std::endl; // сообщение для теста	
	std::cout << "Test message - uri  :" << uri << std::endl; // сообщение для теста	

	std::string origin;
	std::size_t origin_pos = body.find("Origin:");
	if (origin_pos != std::string::npos) {
		std::size_t end_pos = body.find("\r\n", origin_pos);
		origin = trim(body.substr(origin_pos + 7, end_pos - origin_pos - 5));
		std::cout << "origin: [" << origin << "]" << std::endl; // Вывод значения origin
	}

	if (uri.empty() || method.empty() || body.empty() || origin.empty()) {
		closeSocket(socket);
		return;
	}

	if (method == "OPTIONS") {		
		if (corsConfig.handleCors(socket, origin)) {// CORS processing	
			std::cout << "CORS method fertig!" << std::endl; // сообщение для теста	
		}
		else {
			std::cout << "????????????????? CORS method kaput ?????????????????????????????!" << std::endl; // сообщение для теста	
		}
		closeSocket(socket);
		return;
	}
	std::cout << "Weite kodd!!!!" << std::endl; // сообщение для теста	

	if (uri == "/api/sum" && method == "POST") {
		std::size_t json_start = body.find('{');
		std::string json_body = body.substr(json_start);  // Извлечение JSON

		if (!json_body.empty()) {
			std::cout << "Simple Code - 22222222222222222222222222222222222222" << std::endl;
			try {
				std::cout << "json_body--" << json_body << std::endl;

				// Разбор JSON
				json parsed_body = json::parse(json_body);
				std::cout << "parsed_body--" << parsed_body << std::endl;

				// Извлечение числовых значений
				int a = parsed_body["a"];
				int b = parsed_body["b"];
				int sum = a + b;
				std::cout << "Parsed Values - a: " << a << ", b: " << b << ", sum: " << sum << std::endl;

				// Создание JSON-ответа
				json response_json;
				response_json["sum"] = sum;

				// Преобразование JSON в строку
				std::string response_body = response_json.dump();
				std::string response =
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/json\r\n"
					"Content-Length: " + std::to_string(response_body.size()) + "\r\n\r\n" +
					response_body;

				// Отправка ответа с использованием Boost.Asio
				boost::asio::write(socket, boost::asio::buffer(response));
			}
			catch (const std::exception& e) {
				std::cerr << "JSON Parsing error: " << e.what() << std::endl;
			}
		}
		closeSocket(socket);
		return;
	}


	// If the request is for /api/auth/login - we do login,  we add JWT Token
	if (uri == "/api/auth/login" && method == "POST") {
		std::size_t json_start = body.find('{');
		std::string json_body = body.substr(json_start);  // Extract JSON bite
		if (json_body != "") {
			std::cout << "22222222222222222222222222222222222222" << std::endl; // сообщение для теста					
			try {
				std::cout << "json_body--" << json_body << std::endl; // сообщение для теста					
				json parsed_body = json::parse(json_body);  // Parse the body as JSON
				std::cout << "parsed_body--" << parsed_body << std::endl; // сообщение для теста					

				std::string email = parsed_body["email"];
				std::string password = parsed_body["password"];
				std::cout << "Login--" << email << "------" << password << std::endl; // сообщение для теста					

				if (handleDbQuery(socket,
					"SELECT * FROM usersad WHERE email = '" + email + "' AND password = '" + password + "';",
					"Profile data for email: " + email + ", password: " + password + "\n",
					"Login"))
				{
					std::cout << "Login OK" << std::endl; // сообщение для теста
				}
			}
			catch (const std::exception& e) {
				std::cerr << "JSON Parsing error: " << e.what() << std::endl;
			}
		}
		closeSocket(socket);
		return;
	}
	std::cout << "999999999999999" << uri << method << std::endl; // сообщение для теста

	// Process the headers and look for the token  //std::istringstream body_stream(body);
	std::istringstream body_stream(body);
	std::string header;

	std::string token;
	while (std::getline(body_stream, header)) {
		std::cout << "header: " << header << std::endl;//сообщеяие для теста
		if (header.find("{") != std::string::npos) { break; }

		if (header.find("Cookie:") == 0) {
			token = trim(header.substr(21, header.find(';') - 21));
			std::cout << "token: /" << token << std::endl;//сообщеяие для теста
			break;
		}
	}
	if (token.empty() || !validateJWT(socket, token)) {
		std::string response = "HTTP/1.1 401 Unauthorized\r\n";
		response += "Content-Type: text/plain\r\n";
		response += "\r\n";
		response += "Invalid or missing token";

		boost::asio::write(socket, boost::asio::buffer(response));
		closeSocket(socket);
		return;
	}
	// Check if URI starts with "/api/"
	std::string api_prefix = "/api/";
	if (uri.substr(0, api_prefix.length()) != api_prefix) {
		std::string not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
		boost::asio::write(socket, boost::asio::buffer(not_found_response));
		closeSocket(socket);
		return;
	}
	std::unordered_map<std::string, int> uri_map = {
	   {"/api/cart/", 1},
	   {"/api/profile/", 2},
	   {"/api/orders/", 3}
	};

	std::string base_uri = uri.substr(0, uri.find('/', api_prefix.length()) + 1);
	int uri_key = uri_map[base_uri];

	long long codd_id = 0;
	std::cout << "base_uri.length(): " << base_uri.length() << " uri.length(): " << uri.length() << std::endl;//сообщеяие для теста
	std::cout << "base_uri: " << base_uri << " uri: " << uri << std::endl;//сообщеяие для теста

	if (base_uri.length() != uri.length()) {
		try {
			codd_id = std::stoll(uri.substr(base_uri.length()));
		}
		catch (...) {
			closeSocket(socket);
			return; // If out of range
		}
	}
	std::cout << "handleRequest -- uri_key: " << uri_key << " ============= user_id: " << codd_id << std::endl;//сообщеяие для теста

	switch (uri_key) {
	case 1: {  // Processing a Query for a Cart
		handleDbQuery(socket,
			"SELECT * FROM carts WHERE user_id = " + std::to_string(codd_id) + ";",
			"Cart items for user " + std::to_string(codd_id) + ":\n",
			"Json");
		break;
	}
	case 2: {  // Обработка запроса для профиля пользователя
		handleDbQuery(socket,
			"SELECT * FROM users WHERE user_id = " + std::to_string(codd_id) + ";",
			"Profile data for user " + std::to_string(codd_id) + ":\n",
			"Json");
		break;
	}
	case 3: {  // Обработка запроса для заказов пользователя
		handleDbQuery(socket,
			"SELECT * FROM orders WHERE user_id = " + std::to_string(codd_id) + ";",
			"Order details for user " + std::to_string(codd_id) + ":\n",
			"Json");
		break;
	}
	default: {  // Если URI не найден, возвращаем 404
		std::string not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
		boost::asio::write(socket, boost::asio::buffer(not_found_response));
		break;
	}
	}
	// Закрываем соединение после обработки запроса
	closeSocket(socket);
}