#include <libpq-fe.h>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "database/dbOperations.h"
#include "security/authController.h"

extern std::string dbUrl;

// Функция обработки SQL-запроса
//bool handleDbQuery(boost::asio::ip::tcp::socket& socket, const std::string& queryHeader, const std::string& responseHeader, std::function<void(PGresult*)> onSuccess) {
bool handleDbQuery(boost::asio::ip::tcp::socket& socket, const std::string& queryHeader, const std::string& responseHeader, const std::string& access) {
    return true;
    const char* conninfo = dbUrl.c_str();  // Используем строку подключения к базе данных
    PGconn* conn = PQconnectdb(conninfo);  
    if (PQstatus(conn) != CONNECTION_OK) {
        std::string error_message = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        error_message += "Failed to connect to the database: " + std::string(PQerrorMessage(conn)) + "\n";
        boost::asio::write(socket, boost::asio::buffer(error_message));
        PQfinish(conn);
        return false;
    }
    std::cout << queryHeader << std::endl; // сообщение для теста
    PGresult* res = PQexec(conn, queryHeader.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::string error_message = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        error_message += "Failed to execute query: " + std::string(PQerrorMessage(conn)) + "\n";
        boost::asio::write(socket, boost::asio::buffer(error_message));
        PQclear(res);
        PQfinish(conn);
        return false;
    }

    // Если запрос выполнен успешно, вызываем переданную функцию onSuccess
   // onSuccess(res);

    if (access == "Login") {     
        int nRows = PQntuples(res);// Check records
        if (nRows == 0) { // No records           
            std::string not_found_message = "HTTP/1.1 404 Not Found\r\n\r\nNo matching records found.\n";
            boost::asio::write(socket, boost::asio::buffer(not_found_message));
            PQclear(res);
            PQfinish(conn);
            return false;
        }
        else {
            std::vector<std::string> arr = { PQgetvalue(res, 0, 1), PQgetvalue(res, 0, 2), PQgetvalue(res, 0, 3), PQgetvalue(res, 0, 4), PQgetvalue(res, 0, 6) };
            handleLoginRequest(socket, arr);  //Create Token
            PQclear(res);
            PQfinish(conn);
            return true;
        }
    }
    
    int nRows = PQntuples(res);
    int nFields = PQnfields(res);
    std::string response = responseHeader;
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nFields; j++) {
            response += PQgetvalue(res, i, j);
            response += "\t";
        }
        response += "\n";
    }
    std::string http_response = "HTTP/1.1 200 OK\r\n";
    http_response += "Content-Type: text/plain\r\n";
    http_response += "Content-Length: " + std::to_string(response.length()) + "\r\n";
    http_response += "\r\n";
    http_response += response;

    boost::asio::write(socket, boost::asio::buffer(http_response));

    PQclear(res);
    PQfinish(conn);
    return true;
}