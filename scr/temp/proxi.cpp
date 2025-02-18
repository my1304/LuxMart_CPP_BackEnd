#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

void forwardRequest(tcp::socket& client_socket, tcp::socket& server_socket) {
    boost::asio::streambuf client_request;
    boost::asio::read_until(client_socket, client_request, "\r\n\r\n");  // Чтение запроса клиента

    // Отправка запроса серверу (localhost:8080)
    boost::asio::write(server_socket, client_request.data());

    // Чтение ответа от сервера
    boost::asio::streambuf server_response;
    boost::asio::read_until(server_socket, server_response, "\r\n\r\n");

    // Отправка ответа клиенту
    boost::asio::write(client_socket, server_response.data());
}

void startProxy(boost::asio::io_context& io_context) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 3000));  // Запуск прокси на порту 3000

    while (true) {
        tcp::socket client_socket(io_context);
        acceptor.accept(client_socket);  // Ожидание подключения от клиента

        // Подключение к целевому серверу (localhost:8080)
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("localhost", "8080");
        tcp::socket server_socket(io_context);
        boost::asio::connect(server_socket, endpoints);  // Подключение к целевому серверу

        try {
            forwardRequest(client_socket, server_socket);  // Пересылка запроса и получение ответа
        } catch (std::exception& e) {
            std::cerr << "Ошибка при пересылке запроса: " << e.what() << std::endl;
        }

        // Закрытие сокетов после завершения запроса
        client_socket.close();
        server_socket.close();
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        startProxy(io_context);
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}