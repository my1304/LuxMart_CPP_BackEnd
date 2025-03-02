//#pragma once
#include <iostream>
#include <string>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include "database/dbOperations.h"
#include "security/jwtService.h"

namespace http = boost::beast::http;
using json = nlohmann::json;

// Function check Tocken
bool authorizeRequest(const http::request<http::string_body>& req, http::response<http::string_body>& res) {
	auto authHeader = req.base().find(http::field::authorization);
	if (authHeader == req.base().end() || authHeader->value().empty()) {
		res.result(http::status::unauthorized);
		res.body() = "{\"error\":\"Authorization header missing\"}";
		res.set(http::field::content_type, "application/json");
		return false;
	}

	std::string token = std::string(authHeader->value());
	if (token.find("Bearer ") == 0) {
		token = token.substr(7);  // Убираем "Bearer "
	}

	JwtService jwtService;
	if (!jwtService.validateToken(token)) {
		res.result(http::status::unauthorized);
		res.body() = "{\"error\":\"Invalid or expired token\"}";
		res.set(http::field::content_type, "application/json");
		return false;
	}
	return true;
}
// Function list products
void handleProductsRequest(json& response_body, http::response<http::string_body>& res, const json& parsed_body) {
	try {
		if (handleDbQuery("SELECT products.*, ARRAY_AGG(pictures.url) AS image_urls FROM products LEFT JOIN pictures ON "
			"products.id = pictures.product_id GROUP BY products.id;", "FetchProducts", "", response_body, parsed_body)) {
			res.body() = response_body.dump();
			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			std::cout << "Products fetched successfully." << std::endl;
		}
		else {
			std::cerr << "Failed to fetch products." << std::endl;
			res.result(http::status::internal_server_error);
			res.set(http::field::content_type, "application/json");
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error processing categories request: " << e.what() << std::endl;
		res.result(http::status::bad_request);
		res.set(http::field::content_type, "application/json");
	}
}

// main controllers
void controllers(const json& parsed_body, http::response<http::string_body>& res, const http::request<http::string_body>& req) {
	std::string path = std::string(req.target());
	json response_body;

	if (path == "/api/auth/login") {
		try {
			std::string email = parsed_body["email"];
			std::string password = parsed_body["password"];
			if (handleDbQuery("SELECT * FROM usersad WHERE email = '" + email + "';", "Login", password, response_body, parsed_body)) {
				res.body() = response_body.dump();
				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				std::cout << "Login successful for email: " << email << std::endl;
				return;
			}
			else {
				std::cerr << "Invalid login attempt for email: " << email << std::endl;
				res.result(http::status::unauthorized);
				res.set(http::field::content_type, "application/json");
				return;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error processing login request: " << e.what() << std::endl;
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			return;
		}
	}
	if (!authorizeRequest(req, res)) return;

	if (path == "/api/auth/logout") {// Формирование ответа об успешном выходе	
		response_body["message"] = "Logout successful";
		res.body() = response_body.dump();
		res.result(http::status::ok);
		res.set(http::field::content_type, "application/json");
		std::cout << "Logout successful!" << std::endl;
		return;
	}
	if (path == "/api/categories") {
		try {
			if (handleDbQuery("SELECT * FROM categories;", "FetchCategories", "", response_body, parsed_body)) {
				res.body() = response_body.dump();
				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				std::cout << "Categories fetched successfully." << std::endl;
				return;
			}
			else {
				std::cerr << "Failed to fetch categories." << std::endl;
				res.result(http::status::internal_server_error);
				res.set(http::field::content_type, "application/json");
				return;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error processing categories request: " << e.what() << std::endl;
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			return;
		}
	}
	if (path == "/api/products") {
		handleProductsRequest(response_body, res, parsed_body);
		return;
	}
	std::string method = std::string(req.method_string());
	if (path.rfind("/api/category/", 0) == 0) { // Проверка, начинается ли путь с "/api/category/"
		try {
			std::string identifier = path.substr(14); // Извлекаем значение после "/api/category/"
			if (identifier == "add") { // Проверяем, если значение "add"
				if (handleDbQuery("SELECT * FROM categories ORDER BY id ASC;", "FetchCategoryAdd", "", response_body, parsed_body)) {
					res.body() = response_body.dump();
					res.result(http::status::ok);
					res.set(http::field::content_type, "application/json");
					std::cout << "Category fetch for add completed successfully." << std::endl;
					return;
				}
				else {
					std::cerr << "Failed to fetch categories for adding." << std::endl;
					res.result(http::status::internal_server_error);
					res.set(http::field::content_type, "application/json");
					return;
				}
			}
			else { // Простая проверка, что идентификатор состоит только из цифр				
				if (!std::all_of(identifier.begin(), identifier.end(), ::isdigit)) {
					throw std::invalid_argument("Identifier is not numeric.");
				}
				if (method == "DELETE")
				{
					// Удаление категории		
					if (handleDbQuery("DELETE FROM categories WHERE id = " + identifier + ";", "DeleteRecords", "", response_body, parsed_body)) {
						response_body["message"] = "Category deleted successfully";
						res.body() = response_body.dump();
						res.result(http::status::ok);
						res.set(http::field::content_type, "application/json");
						std::cout << "Category deleted with ID: " << identifier << std::endl;
						return;
					}
					else {
						res.result(http::status::internal_server_error);
						res.body() = "{\"error\":\"Failed to delete category\"}";
						res.set(http::field::content_type, "application/json");
						std::cerr << "Failed to delete category with ID: " << identifier << std::endl;
						return;
					}
				}
				else if (method == "PUT") {
					if (handleDbQuery("SELECT * FROM categories WHERE id = " + identifier + ";",
						"FetchCategoryUpdate", "", response_body, parsed_body)) {
						res.body() = response_body.dump();
						res.result(http::status::ok);
						res.set(http::field::content_type, "application/json");
						std::cout << "Category fetch for update completed successfully." << std::endl;
						return;
					}
					else {
						std::cerr << "Failed to fetch category with ID: " << identifier << std::endl;
						res.result(http::status::internal_server_error);
						res.set(http::field::content_type, "application/json");
						return;
					}
				}
			}
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Invalid category identifier: " << path.substr(14) << ". Error: " << e.what() << std::endl;
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			return;
		}
		catch (const std::exception& e) {
			std::cerr << "Error processing category request: " << e.what() << std::endl;
			res.result(http::status::internal_server_error);
			res.set(http::field::content_type, "application/json");
			return;
		}
	}
	if (path.rfind("/api/product/", 0) == 0) { // Проверка, начинается ли путь с  "/api/product/"
		try {
			std::string identifier = path.substr(13); // Извлекаем значение после "/api/product/"
			if (identifier == "add") { // Проверяем, если значение "add"
				if (handleDbQuery("SELECT * FROM products ORDER BY id ASC;", "FetchProductsAdd", "", response_body, parsed_body)) {
					res.body() = response_body.dump();
					res.result(http::status::ok);
					res.set(http::field::content_type, "application/json");
					std::cout << "Products fetch for add completed successfully." << std::endl;
					return;
				}
				else {
					std::cerr << "Failed to fetch products for adding." << std::endl;
					res.result(http::status::internal_server_error);
					res.set(http::field::content_type, "application/json");
					return;
				}
			}
			else { // Простая проверка, что идентификатор состоит только из цифр
				if (!std::all_of(identifier.begin(), identifier.end(), ::isdigit)) {
					throw std::invalid_argument("Identifier is not numeric.");
				}
				if (method == "DELETE")
				{
					if (handleDbQuery("DELETE FROM pictures WHERE product_id = " + identifier + ";", "DeleteRecords", "", response_body, parsed_body)) {
						if (!handleDbQuery("DELETE FROM products WHERE id = " + identifier + ";", "DeleteRecords", "", response_body, parsed_body)) {
							res.result(http::status::internal_server_error);
							res.body() = "{\"error\":\"Failed to delete product\"}";
							res.set(http::field::content_type, "application/json");
							std::cerr << "Failed to delete product with ID: " << identifier << std::endl;
							return;
						}
						res.body() = response_body.dump();
						res.result(http::status::ok);
						res.set(http::field::content_type, "application/json");
						return;
					}
					else {
						res.result(http::status::internal_server_error);
						res.body() = "{\"error\":\"Failed to delete product\"}";
						res.set(http::field::content_type, "application/json");
						std::cerr << "Failed to delete pictures with ID: " << identifier << std::endl;
						return;
					}
				}
				if (method == "PUT") {
					if (!handleDbQuery("DELETE FROM pictures WHERE product_id = " + identifier + ";", "DeleteRecords", "", response_body, parsed_body)) {
					//	res.result(http::status::internal_server_error);
					//	res.body() = "{\"error\":\"Failed to delete product\"}";
					//	res.set(http::field::content_type, "application/json");
						std::cerr << "-----------Failed ----- to ---- delete ---- product with ID: " << identifier << std::endl;
					//	return;
					}
					if (handleDbQuery("SELECT * FROM products WHERE id = " + identifier + ";",
						"FetchProductUpdate", "", response_body, parsed_body)) {
						res.body() = response_body.dump();
						res.result(http::status::ok);
						res.set(http::field::content_type, "application/json");
						std::cout << "Products fetch for update completed successfully." << std::endl;
						return;
					}
					else {
						std::cerr << "Failed to fetch category with ID: " << identifier << std::endl;
						res.result(http::status::internal_server_error);
						res.set(http::field::content_type, "application/json");
						return;
					}
				}
			}
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Invalid category identifier: " << path.substr(14) << ". Error: " << e.what() << std::endl;
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			return;
		}
		catch (const std::exception& e) {
			std::cerr << "Error processing category request: " << e.what() << std::endl;
			res.result(http::status::internal_server_error);
			res.set(http::field::content_type, "application/json");
			return;
		}
	}
}