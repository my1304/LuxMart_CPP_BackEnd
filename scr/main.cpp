#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include "security/security.h"
#include <nlohmann/json.hpp>

using boost::asio::ip::tcp;
namespace net = boost::asio;
using json = nlohmann::json;

json configData = readConfig();

int serverPort = configData["server"]["port"];
//std::string dbUrl = "dbname = luxmart_db user = postgres password = qwerty007 hostaddr = 127.0.0.1 port = 5432";
//std::string dbUrl = "dbname = luxmart_db user = postgres password = Qwerty!123 hostaddr = 10.1.1.18 port = 5432";
std::string dbUrl = "dbname = " + configData["database"]["name"].get<std::string>() +
" user = " + configData["database"]["username"].get<std::string>() +
" password = " + configData["database"]["password"].get<std::string>() +
" host = " + configData["database"]["host"].get<std::string>();

std::string pictureUrl = R"(\\COMPSL\www)";

bool is_ip_in_range(const std::string& ip_str) {
	using namespace boost::asio::ip;

	try {
		address ip = make_address(ip_str);
		address start = make_address("0.0.0.0");
		address end = make_address("255.255.255.255");

		if (ip.is_v4() && start.is_v4() && end.is_v4()) {
			auto ipv4 = ip.to_v4().to_uint();
			return (ipv4 >= start.to_v4().to_uint())
				&& (ipv4 <= end.to_v4().to_uint());
		}
	}
	catch (...) {
		return false;
	}
	return false;
}

bool check_range(tcp::socket& socket) { // Передаем сокет по ссылке
	try {
		auto remote_ip = socket.remote_endpoint().address().to_string();
		std::cout << "Incoming connection from IP: " << remote_ip << std::endl; // out screen IP
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
    if (configData["database"]["port"].get<int>() != 0) {
        dbUrl += " port = " + std::to_string(configData["database"]["port"].get<int>());
    }
	try {
		auto const address = net::ip::make_address("0.0.0.0");
		unsigned short port = serverPort;

		net::io_context ioc{ 1 };
		tcp::acceptor acceptor{ ioc, {address, port} };
		std::cout << "dbUrl: " << dbUrl << std::endl; // out screen dbUrl
		std::cerr << "Server is listening on " << address << ":" << port << std::endl;


		net::thread_pool pool(4); // Пул потоков

		for (;;) {
			tcp::socket socket{ ioc };
			acceptor.accept(socket);
			if (check_range(socket)) {
				net::post(pool, [socket = std::move(socket)]() mutable {
					do_session(std::move(socket));
					});
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}