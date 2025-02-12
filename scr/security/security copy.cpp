#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include "controllers/controllers.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

void closeSocket(tcp::socket& socket) {
	boost::system::error_code error;
	socket.shutdown(tcp::socket::shutdown_both, error);
	if (!error) {
		socket.close();
		std::cout << "Socket closed!" << std::endl;
	}
}

template <class Body, class Allocator, class Send>
bool handle_request(const http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
	http::response<http::string_body> res;

	// ��������� CORS-���������� � ������������ ��������� Origin � ��������� �������
	res.set(http::field::access_control_allow_origin, req.base().find(http::field::origin) != req.base().end() ? std::string(req[http::field::origin]) : "*");// ��������� CORS-���������� � ������������ ��������� Origin
	res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
	res.set(http::field::access_control_allow_headers, "Authorization, Content-Type");

	// ��������� CORS-��������
	if (req.method() == http::verb::options) {
		res.result(http::status::no_content);
		res.prepare_payload();
		send(std::move(res));
		return true;
	}
	
	if (req.method() != http::verb::get && req.body().empty()) {
		res.result(http::status::bad_request);
		res.body() = "Request body is empty!";
		send(std::move(res));
		return false;
	}

	try {
		json parsed_body = json::parse(req.body().empty() ? "{}" : req.body()); // ������ ���� �������
		controllers(parsed_body, res, req);// �������� ������ � ����������
	}
	catch (const json::parse_error& e) {
		res.result(http::status::bad_request);
		res.body() = std::string("JSON parse error: ") + e.what();
		send(std::move(res));
		return false;
	}
	res.prepare_payload();
	send(std::move(res));
	return true;
}

void do_session(tcp::socket socket) {
	beast::flat_buffer buffer;
	boost::system::error_code ec;

	for (;;) {
		http::request<http::string_body> req;
		http::read(socket, buffer, req, ec);

		if (ec == http::error::end_of_stream)
			break;
		if (ec)
			return;

		auto const send = [&](auto&& response) {
			using response_type = typename std::decay<decltype(response)>::type;
			auto sp = std::make_shared<response_type>(std::forward<decltype(response)>(response));
			http::write(socket, *sp, ec);
			return ec;
			};

		//if (!handle_request(req, send))
			break;		
	}
	closeSocket(socket);
}