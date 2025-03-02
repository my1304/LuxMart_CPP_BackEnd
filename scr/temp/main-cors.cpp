#define _WIN32_WINNT 0x0601  // Поддержка Windows 7 и выше

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <nlohmann/json.hpp>  // Библиотека для обработки JSON, используем nlohmann/json.hpp
#include <iostream>
#include <string>

std::string dbUrl = "dbname = luxmart_db user = postgres password = qwerty007 hostaddr = 127.0.0.1 port = 5432";
int serverPort = 8080;

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json; // Определение для удобства работы с JSON


template <class Body, class Allocator, class Send>
void handle_request(const http::request<Body, http::basic_fields<Allocator>>& req, Send&& send) {
    http::response<http::string_body> res;

    std::cerr << "111111111111111111111111111: " << std::endl;
    // Устанавливаем CORS-заголовки
    res.set(http::field::access_control_allow_origin, "*");
    res.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
    res.set(http::field::access_control_allow_headers, "Content-Type");

    // Обработка запроса OPTIONS
    if (req.method() == http::verb::options) {
        std::cerr << "2222222222222222222222222: " << std::endl;
        res.result(http::status::no_content);
        send(std::move(res));
        return;
    }

    // Обработка POST-запроса на /api/sum
    if (req.method() == http::verb::post && req.target() == "/api/sum") {
        std::cerr << "3333333333333333333333333333333333: " << std::endl;
        try {
            // Чтение и парсинг JSON из тела запроса
            auto body = req.body();
            auto parsed_body = json::parse(body);

            int a = parsed_body["a"];
            int b = parsed_body["b"];
            int sum = a + b;

            // Формируем JSON-ответ
            json response_body;
            response_body["sum"] = sum;
            res.body() = response_body.dump();
            res.result(http::status::ok);
            res.set(http::field::content_type, "application/json");
            std::cerr << "444444444444444444: " << std::endl;
        }
        catch (const std::exception& e) {
            // Обработка ошибок парсинга JSON
            res.result(http::status::bad_request);
            res.body() = "Ошибка при обработке JSON";
            res.set(http::field::content_type, "text/plain");
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }
    else {
        // Если запрос не соответствует ни одному из условий
        std::cerr << "fehle--------------: " << std::endl;
        res.result(http::status::not_found);
        res.body() = "Endpoint not found";
        res.set(http::field::content_type, "text/plain");
    }

    res.prepare_payload();
    send(std::move(res));
}

void do_session(tcp::socket socket) {
    beast::flat_buffer buffer;
    boost::system::error_code ec;  // Объявляем ec как error_code

    for (;;) {
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);

        if (ec == http::error::end_of_stream)
            break;
        if (ec)
            return;

        // Ответ на запрос
        auto const send = [&](auto&& response) {
            using response_type = typename std::decay<decltype(response)>::type;
            auto sp = std::make_shared<response_type>(std::forward<decltype(response)>(response));
            http::write(socket, *sp, ec);
            return ec;
            };

        handle_request(req, send);
    }

    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    try {
        auto const address = net::ip::make_address("0.0.0.0");
        unsigned short port = serverPort;

        net::io_context ioc{ 1 };
        tcp::acceptor acceptor{ ioc, {address, port} };

        std::cerr << "Load: " << address << "--" << port << std::endl;

        for (;;) {
            tcp::socket socket{ ioc };
            acceptor.accept(socket);
            std::thread(do_session, std::move(socket)).detach();
        }
    }
    catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}