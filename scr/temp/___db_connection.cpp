// Файл подключения к базе данных PostgreSQL

// LuxMart_CPP_BackEnd.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <libpq-fe.h>

int db_connection()
{

 


    // Строка подключения: укажите параметры вашей базы данных
    const char* conninfo = "dbname = luxmart_db user = postgres password = qwerty007 hostaddr = 127.0.0.1 port = 5432";

    // Открываем соединение с базой данных
    PGconn* conn = PQconnectdb(conninfo);

    // Проверяем статус подключения
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return 1;
    }
    std::cout << "Connected to the database successfully!" << std::endl;

    // Выполняем SQL-запрос
    PGresult* res = PQexec(conn, "SELECT * FROM users");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SELECT failed: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        PQfinish(conn);
        return 1;
    }

    // Обрабатываем результаты запроса
    int nRows = PQntuples(res);
    int nFields = PQnfields(res);

    std::cout << "Number of rows: " << nRows << std::endl;

    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nFields; j++) {
            std::cout << PQgetvalue(res, i, j) << "\t";
        }
        std::cout << std::endl;
    }

    // Освобождаем память и закрываем соединение
    PQclear(res);
    PQfinish(conn);

    return 0;


    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
