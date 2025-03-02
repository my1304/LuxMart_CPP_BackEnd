#include <boost/asio.hpp>
#include <libpq-fe.h>
#include <iostream>
#include <string>

void authenticateUser(tcp::socket& socket, const std::string& user_id, const std::string& password) {
    const char* conninfo = "your_database_url";
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        std::string error_message = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        error_message += "Failed to connect to the database: " + std::string(PQerrorMessage(conn)) + "\n";
        boost::asio::write(socket, boost::asio::buffer(error_message));
        PQfinish(conn);
        return;
    }

    std::string query = "SELECT * FROM users WHERE user_id = '" + user_id + "' AND password = '" + password + "';";
    PGresult* res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        std::string error_message = "HTTP/1.1 401 Unauthorized\r\n\r\n";
        error_message += "Invalid credentials\n";
        boost::asio::write(socket, boost::asio::buffer(error_message));
        PQclear(res);
        PQfinish(conn);
        return;
    }

    // Создаем сессию для пользователя (можно использовать JWT или другой метод сессий)
    std::string session_id = "session_token_here"; // Например, генерируем токен
    std::string success_message = "HTTP/1.1 200 OK\r\n\r\nSession created for user " + user_id + "\n";
    boost::asio::write(socket, boost::asio::buffer(success_message));

    PQclear(res);
    PQfinish(conn);
}