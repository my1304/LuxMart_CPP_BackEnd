#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

void handleClient(tcp::socket& socket) {
    try {
        // ������ ������� �������
        boost::asio::streambuf request;
        boost::asio::read_until(socket, request, "\r\n\r\n");

        // ��������� ������� (��� ������� ������ ������� JSON-�����)
        std::string response_body = "{\"message\": \"Hello from C++ server!\"}";

        // ������������ � �������� ������
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"  // ���������� ������� ��� ���� �������
            "Content-Length: " + std::to_string(response_body.size()) + "\r\n\r\n" +
            response_body;

        boost::asio::write(socket, boost::asio::buffer(response));
    }
    catch (std::exception& e) {
        std::cerr << "������ ��� ��������� �������: " << e.what() << std::endl;
    }
}

void startServer(boost::asio::io_context& io_context) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));  // ������� �� ����� 8080

    while (true) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);  // �������� ����������� �� �������
        handleClient(socket);     // ��������� �������
        socket.close();           // �������� ����������
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        startServer(io_context);  // ������ �������
    }
    catch (std::exception& e) {
        std::cerr << "������: " << e.what() << std::endl;
    }

    return 0;
}