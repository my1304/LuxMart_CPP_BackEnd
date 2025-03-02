#ifndef DB_OPERATIONS_H
#define DB_OPERATIONS_H

#include <boost/asio.hpp>
#include <libpq-fe.h>
#include <string>
#include <nlohmann/json.hpp>  // Подключаем библиотеку для работы с JSON

using json = nlohmann::json;


bool handleDbQuery(const std::string& queryHeader, const std::string& access, const std::string& operation, json& response_json, const json& parsed_body);

#endif  // DB_OPERATIONS_H