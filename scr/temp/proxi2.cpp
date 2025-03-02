#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

void handleClient(tcp::socket& socket) {
    try {
        // Чтение запроса клиента
        boost::asio::streambuf request;
        boost::asio::read_until(socket, request, "\r\n\r\n");

        // Обработка запроса (для примера создаём простой JSON-ответ)
        std::string response_body = "{\"message\": \"Hello from C++ server!\"}";

        // Формирование и отправка ответа
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"  // Разрешение доступа для всех доменов
            "Content-Length: " + std::to_string(response_body.size()) + "\r\n\r\n" +
            response_body;

        boost::asio::write(socket, boost::asio::buffer(response));
    }
    catch (std::exception& e) {
        std::cerr << "Ошибка при обработке клиента: " << e.what() << std::endl;
    }
}

void startServer(boost::asio::io_context& io_context) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));  // Слушаем на порту 8080

    while (true) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);  // Ожидание подключения от клиента
        handleClient(socket);     // Обработка клиента
        socket.close();           // Закрытие соединения
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        startServer(io_context);  // Запуск сервера
    }
    catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

    return 0;
}