#include "security/security.h"
#include <boost/asio.hpp>
#include <set>
#include <string>
#include <iostream>
#include <utility>  // ��� std::pair

using boost::asio::ip::tcp;

// ������� ��� ���������� ����� ������� � ����� �� origin
std::pair<std::string, std::string> serverNamePort(const std::string& origin) {
	std::string server_name;
	std::string port;

	// ������� ������ ����� ������� ����� "http://"
	std::size_t protocol_pos = origin.find("://");
	std::size_t server_start = (protocol_pos == std::string::npos) ? (0) : (protocol_pos + 3);
	std::size_t port_start = origin.find(":", server_start);  // ������ �����

	// ���������� ����� �������
	if (port_start != std::string::npos) {
		server_name = origin.substr(server_start, port_start - server_start);

		// ���������� ������ �����
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
	return { "", "" };// ���������� ������ ����, ���� �� ������� ������� ��� ������� � ����
}

bool sendToFrontend(const std::string& response, const std::string& origin) {
	try {
		auto server_info = serverNamePort(origin);  // �������� ���� {server_name, port}
		std::cerr << "sendToFrontend----origin_name [" << server_info.first << "]  -- [" << server_info.second << "]" << std::endl;
		std::cerr << "response: " << response << std::endl;
		boost::asio::io_context io_context;
		// ��������� ������ � �����
		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve(server_info.first, server_info.second);

		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);

		boost::asio::write(socket, boost::asio::buffer(response)); // �������� ������ �� ��������� ����� � ����
		return true;
	}
	catch (std::exception& e) {
		std::cerr << "������ ��� �������� ������: " << e.what() << std::endl;
		return false;
	}
}

// ���������� ������ ��� ��������� CORS
bool CorsConfig::handleCors(tcp::socket& socket, const std::string& origin) {

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: application/json\r\n";
	response += "Access-Control-Allow-Origin: " + origin + "\r\n";
	response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
	response += "Access-Control-Allow-Headers: Authorization, Content-Type\r\n";
	response += "Access-Control-Allow-Credentials: true\r\n";
	response += "Access-Control-Max-Age: 86400\r\n"; // ����������� preflight ������� �� 1 ����
	response += "\r\n";

	// ���������� ����� �� ��������� ������ � ����
	return sendToFrontend(response, origin);
}



/*
// ���������� ������� ��� �������� ����������� ������� � CORS
bool authorizeRequestWithCors(tcp::socket& socket, const std::string& session_id) {
	// �������� ������
	if (session_id != "valid_session_token") {
		std::string error_response = "HTTP/1.1 403 Forbidden\r\n\r\nInvalid session\n";
		boost::asio::write(socket, boost::asio::buffer(error_response));
		return false;
	}

	// ��������� CORS
	CorsConfig cors;
	return cors.handleCors(socket);
}
*/