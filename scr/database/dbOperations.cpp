#include <libpq-fe.h>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "database/dbOperations.h"
#include "security/authController.h"
#include "services/functions.h"

extern std::string dbUrl;
using json = nlohmann::json;

bool handleDbQuery(const std::string& queryHeader, const std::string& access, const std::string& operation, json& response_json, const json& parsed_body) {
	const char* conninfo = dbUrl.c_str();
	PGconn* conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK) {
		response_json["status"] = "error";
		response_json["message"] = "Failed to connect to the database: " + std::string(PQerrorMessage(conn));
		PQfinish(conn);
		return false;
	}

	PGresult* res = PQexec(conn, queryHeader.c_str());

	if (access == "DeleteRecords") {
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			response_json["status"] = "error";
			response_json["message"] = "Failed to execute DELETE query: " + std::string(PQerrorMessage(conn));
			PQclear(res);
			PQfinish(conn);
			std::cout << "Delete problem PQresultStatus(res) != PGRES_COMMAND_OK!!!" << std::endl;
			return false;
		}		
		response_json["status"] = "success";
		response_json["message"] = "Records deleted successfully.";
		PQclear(res);
		PQfinish(conn);
		return true;
	}
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		response_json["status"] = "error";
		response_json["message"] = "Failed to execute query: " + std::string(PQerrorMessage(conn));
		PQclear(res);
		PQfinish(conn);
		return false;
	}
	int nRows = PQntuples(res);// Проверка записей
	if (access == "Login") {
		if (nRows == 0) {
			response_json["status"] = "error";
			response_json["message"] = "No matching records found.";
			PQclear(res);
			PQfinish(conn);
			return false;
		}
		else {
			if (matches(operation, PQgetvalue(res, 0, 5))) {
				std::vector<std::string> user_data = {
					PQgetvalue(res, 0, 1),  // username
					PQgetvalue(res, 0, 2),  // first_name
					PQgetvalue(res, 0, 3),  // last_name
					PQgetvalue(res, 0, 4),  // email
					PQgetvalue(res, 0, 6)   // roles
				};
				response_json = handleLoginRequest(user_data);  // Получение JSON-ответа с токенами
				PQclear(res);
				PQfinish(conn);
				return true;
			}
			else {
				response_json["status"] = "error";
				response_json["message"] = "Incorrect password.";
				PQclear(res);
				PQfinish(conn);
				return false;
			}
		}
	}
	if (access == "FetchCategoryUpdate" || access == "FetchCategoryAdd") {
		if (access == "FetchCategoryUpdate" && nRows == 0) {
			response_json["status"] = "error";
			response_json["message"] = "No matching records found.";
			PQclear(res);
			PQfinish(conn);
			return false;
		}
		std::string id_str;
		if (access == "FetchCategoryAdd") {
			int id = 1; 
			for (int i = 0; i < nRows; i++) {	
				if (id < std::stoi(PQgetvalue(res, i, 0))) break; // Если находим пропуск, останавливаемся
				id++;
			}
			id_str = std::to_string(id); // Преобразуем ID в строку для использования в запросе
		}
		else {
			id_str = parsed_body["id"].get<std::string>();
		}

		PQclear(res);
		
		try {
			std::string query = "INSERT INTO categories (id, name, image) VALUES (" + id_str + ", '" +
				parsed_body["name"].get<std::string>() + "', '" +
				parsed_body["image"].get<std::string>() + "') ON CONFLICT (id) DO UPDATE SET " +
				"name = EXCLUDED.name, image = EXCLUDED.image;";

			PGresult* update_res = PQexec(conn, query.c_str());			
			if (PQresultStatus(update_res) != PGRES_COMMAND_OK) {
				response_json["status"] = "error";
				response_json["message"] = "Failed to update or insert record: " + std::string(PQerrorMessage(conn));
				PQclear(update_res);
				PQfinish(conn);
				return false;
			}
			PQclear(update_res);
		}
		catch (const std::exception& e) {
			response_json["status"] = "error";
			response_json["message"] = "Error processing request: " + std::string(e.what());
			std::cerr << "Error processing request: " << e.what() << std::endl;
			PQfinish(conn);
			return false;
		}
		response_json["status"] = "success";
		response_json["message"] = "Record added or updated successfully.";
		response_json["id"] = id_str; // Возвращаем ID
		PQfinish(conn);
		return true;
	}
	if (access == "FetchProductUpdate" || access == "FetchProductsAdd") {

		if (access == "FetchProductUpdate" && nRows == 0) {
			response_json["status"] = "error";
			response_json["message"] = "No matching records found.";
			PQclear(res);
			PQfinish(conn);
			std::cout << "Papal No Record!!!" << std::endl;
			return false;
		}
		std::string id_str;
		if (access == "FetchProductsAdd") {
			int id = 1;
			for (int i = 0; i < nRows; i++) {
				if (id < std::stoi(PQgetvalue(res, i, 0))) break; // Если находим пропуск, останавливаемся
				id++;
			}
			id_str = std::to_string(id); // Преобразуем ID в строку для использования в запросе
		}
		else {
			id_str = parsed_body["id"].get<std::string>();
		}
		PQclear(res);		
		try {	
			std::string query = "INSERT INTO products (id, category_id, title, price, description) VALUES (" + id_str + ", " +
				parsed_body.value("category_id", "0") + ", '" +
				parsed_body.value("title", "Default title") + "', " +
				parsed_body.value("price", "0.00") + ", '" +
				parsed_body.value("description", "No description") + "') ON CONFLICT (id) DO UPDATE SET " +
				"category_id = EXCLUDED.category_id, title = EXCLUDED.title, price = EXCLUDED.price, description = EXCLUDED.description;";
			std::cout << "Test query|: "<< query << std::endl; 

			PGresult* update_res = PQexec(conn, query.c_str());
			if (PQresultStatus(update_res) != PGRES_COMMAND_OK) {
				response_json["status"] = "error";
				response_json["message"] = "Failed to update or insert record: " + std::string(PQerrorMessage(conn));
				PQclear(update_res);
				PQfinish(conn);
				return false;
			}
			PQclear(update_res);
			std::cout << "Image array: " << parsed_body["image"] << std::endl; //пока пусть побудет

			if (!parsed_body.contains("image") || !parsed_body["image"].is_array()) {
				response_json["status"] = "error";
				response_json["message"] = "Invalid or missing 'image' field.";
				std::cout << "Papal No parsed_body!!!" << std::endl;
				PQfinish(conn);
				return false;
			}
			for (const auto& image_url : parsed_body["image"]) {
				if (!image_url.is_string() || image_url.get<std::string>().rfind("http://localhost/", 0) != 0)
					continue;

				PGresult* add_res = PQexec(conn, "SELECT id FROM pictures ORDER BY id ASC;");
				if (PQresultStatus(add_res) != PGRES_TUPLES_OK) continue;
				int id = 1;
				for (int i = 0; i < PQntuples(add_res); i++) {
					if (id < std::stoi(PQgetvalue(add_res, i, 0))) break;
					id++;
				}
				PQclear(add_res);
				std::string query = "INSERT INTO pictures (id, url, product_id) VALUES (" +
					std::to_string(id) + ", '" + image_url.get<std::string>() + "', '" +
					id_str + "') ON CONFLICT (id) DO NOTHING;"; // Пропускаем, если запись уже существует
				PGresult* insert_res = PQexec(conn, query.c_str());
				if (PQresultStatus(insert_res) != PGRES_COMMAND_OK) {
					std::cerr << "Failed to insert image: " << PQerrorMessage(conn) << std::endl;
					PQclear(insert_res);
					continue;
				}
				PQclear(insert_res);
			}
			response_json["status"] = "success";
			response_json["message"] = "Images processed and inserted successfully.";
		}
		catch (const std::exception& e) {
			response_json["status"] = "error";
			response_json["message"] = "Error processing request: " + std::string(e.what());
			std::cerr << "Error processing request: " << e.what() << std::endl;
			PQfinish(conn);
			return false;
		}
		response_json["status"] = "success";
		response_json["message"] = "Record added or updated successfully.";
		response_json["id"] = id_str; // Возвращаем ID
		PQfinish(conn);
		return true;
	}
	//================================================================
	int nFields = PQnfields(res);
	json data_json = json::array();

	for (int i = 0; i < nRows; i++) {
		json row_json;
		for (int j = 0; j < nFields; j++) {
			row_json[PQfname(res, j)] = PQgetvalue(res, i, j);
		}
		data_json.push_back(row_json);
	}

	response_json["status"] = "success";
	response_json["message"] = "Query executed successfully";
	response_json["data"] = data_json;

	PQclear(res);
	PQfinish(conn);
	return true;
}