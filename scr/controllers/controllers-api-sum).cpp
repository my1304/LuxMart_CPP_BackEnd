//#pragma once
#include <iostream>
#include <string>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include "database/dbOperations.h"
#include "security/jwtService.h"

namespace http = boost::beast::http;
using json = nlohmann::json;

void controllers(const json& parsed_body, http::response<http::string_body>& res, const http::request<http::string_body>& req) {
	std::string path = std::string(req.target());

	if (path == "/api/auth/login") {
		try {
			std::string email = parsed_body["email"];
			std::string password = parsed_body["password"];
			json response_body;
			if (handleDbQuery("SELECT * FROM usersad WHERE email = '" + email + "';", "Login", password, response_body)) {
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
	else if (path == "/api/auth/logout") {
		try {
			auto authHeader = req.base().find(http::field::authorization); // Извлечение токена из заголовка Authorization
			if (authHeader == req.base().end() || authHeader->value().empty()) {
				throw std::runtime_error("Authorization header missing");
			}
			std::string token = std::string(authHeader->value()); // Извлечение токена из заголовка (Bearer token)
			if (token.find("Bearer ") == 0) {
				token = token.substr(7);  // "Bearer Weg "
			}

			JwtService jwtService;
			if (!jwtService.validateToken(token)) {
				res.result(http::status::unauthorized);
				res.body() = "{\"error\":\"Invalid or expired token\"}";
				res.set(http::field::content_type, "application/json");
				return;
			}

			// Формирование ответа об успешном выходе
			json response_body;
			response_body["message"] = "Logout successful";
			res.body() = response_body.dump();
			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			std::cout << "Logout successful!" << std::endl;
			return;
		}
		catch (const std::exception& e) {
			res.result(http::status::unauthorized);
			res.body() = "{\"error\":\"Unauthorized request\"}";
			res.set(http::field::content_type, "application/json");
			std::cerr << "Error processing logout request: " << e.what() << std::endl;
			return;
		}

	}
	else if (path == "/api/categories") {
		try {
			json response_body;
			if (handleDbQuery("SELECT * FROM categories;", "FetchCategories", "", response_body)) {
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
	else if (path == "/api/products") {
		try {
			json response_body;
			//if (handleDbQuery("SELECT products.*, pictures.url AS picture_url FROM products LEFT JOIN (SELECT DISTINCT ON(product_id) * FROM pictures ORDER BY product_id, id) AS pictures ON products.id = pictures.product_id;", "FetchProducts", "", response_body)) {
			
			
			if (handleDbQuery("SELECT products.*, ARRAY_AGG(pictures.url) AS image_urls FROM products LEFT JOIN pictures ON "
				"products.id = pictures.product_id GROUP BY products.id;", "FetchProducts", "", response_body)) {
				res.body() = response_body.dump();
				res.result(http::status::ok);
				res.set(http::field::content_type, "application/json");
				std::cout << "Products fetched successfully." << std::endl;
				return;
			}
			else {
				std::cerr << "Failed to fetch products." << std::endl;
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
	else if (path == "/api/sum") {
		try {
			int a = parsed_body["a"];
			int b = parsed_body["b"];
			int sum = a + b;

			json response_body;
			response_body["sum"] = sum;
			res.body() = response_body.dump();
			res.result(http::status::ok);
			res.set(http::field::content_type, "application/json");
			return;
		}
		catch (const std::exception& e) {
			std::cerr << "JSON parse error in /api/sum: " << e.what() << std::endl;
			res.result(http::status::bad_request);
			res.set(http::field::content_type, "application/json");
			return;
		}
	}
}