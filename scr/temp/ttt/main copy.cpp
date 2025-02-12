#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include "security/security.h"

using boost::asio::ip::tcp;
namespace net = boost::asio;

// Глобальные переменные для конфигурации с жестко заданными значениями
std::string dbUrl = "dbname = luxmart_db user = postgres password = Qwerty!123 hostaddr = 10.1.1.18 port = 5432";
std::string pictureUrl = "";
//std::string pictureUrl = "\\COMPSL\www";
int serverPort = 8088;

bool is_ip_in_range(const std::string& ip) {
	std::string range_start = "10.1.1.18";
	std::string range_end = "10.1.1.19";
	return (ip >= range_start && ip <= range_end);
}

bool check_range(tcp::socket& socket) { // Передаем сокет по ссылке
	try {
		auto remote_ip = socket.remote_endpoint().address().to_string();
		if (is_ip_in_range(remote_ip)) {
			return true;
		}
		std::cout << "IP " << remote_ip << " is out of the allowed range. Closing connection." << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "Session error: " << e.what() << std::endl;
	}
	closeSocket(socket); // Закрытие оригинального сокета
	return false;
}

int main() {
	try {
		auto const address = net::ip::make_address("0.0.0.0");
		unsigned short port = serverPort;

		net::io_context ioc{ 1 };
		tcp::acceptor acceptor{ ioc, {address, port} };

		std::cerr << "Server is listening on " << address << ":" << port << std::endl;

		for (;;) {
			tcp::socket socket{ ioc };
			acceptor.accept(socket);
			if (check_range(socket)) {
				std::thread(do_session, std::move(socket)).detach(); // Обработка каждого подключения в отдельном потоке
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}