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
void handle_request(const http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
	http::response<http::string_body> res;

	// Установка CORS-заголовков с динамическим значением Origin с Проверкой наличия
	res.set(http::field::access_control_allow_origin, req.base().find(http::field::origin) != req.base().end() ? std::string(req[http::field::origin]) : "*");// Установка CORS-заголовков с динамическим значением Origin
	res.set(http::field::access_control_allow_methods, "GET, POST, PUT, DELETE, OPTIONS");
	res.set(http::field::access_control_allow_headers, "Authorization, Content-Type");

	// Обработка CORS-запросов
	if (req.method() == http::verb::options) {
		res.result(http::status::no_content);
		send(std::move(res));
		return;
	}

	// Установка тела запроса с проверкой на пустоту
	auto body = req.body().empty() ? "" : req.body();
	json parsed_body;

	// Проверка метода запроса и парсинг тела для POST и PUT
	if (req.method() == http::verb::post || req.method() == http::verb::put) {
		if (body.empty()) {
			res.result(http::status::bad_request);
			res.body() = "Request body is empty for POST/PUT request";
			send(std::move(res));
			return;
		}

		try {
			parsed_body = json::parse(body); // Парсим тело запроса
		}
		catch (const json::parse_error& e) {
			res.result(http::status::bad_request);
			res.body() = std::string("JSON parse error: ") + e.what();
			send(std::move(res));
			return;
		}
	}
	else {
		parsed_body = json::object(); // Устанавливаем пустой JSON объект для GET и других методов
	}

	// Передача данных в контроллер
	controllers(parsed_body, res, req);

	res.prepare_payload();
	send(std::move(res));
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

		handle_request(req, send);
	}

	closeSocket(socket);
}