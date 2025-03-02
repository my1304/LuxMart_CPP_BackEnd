#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "database/dbOperations.h"  
#include "security/authController.h"
#include "services/trim.h"

using boost::asio::ip::tcp;
using json = nlohmann::json;

// Hash function to get a unique integer value for a string
std::size_t hash_uri(const std::string& uri) {
    return std::hash<std::string>{}(uri);
}

// Function that processes requests depending on URI
void handleRequest(tcp::socket& socket) {
    boost::system::error_code error;
    boost::asio::streambuf request;
    boost::asio::read_until(socket, request, "\r\n\r\n", error);
    if (error) {
        std::cerr << "Failed to read request: " << error.message() << std::endl;
        return;
    }

    // Convert the entire request to a string (including headers and body)
    std::istream request_stream(&request);
    std::string method, uri, http_version;
    request_stream >> method >> uri >> http_version;
    std::ostringstream request_body_stream;
    request_body_stream << request_stream.rdbuf();  // Read everything into one buffer
    std::string body = request_body_stream.str();   // Read everything into a string

    // If the request is for /api/auth/login - we do login,  we add JWT Token
	if (uri == "/api/auth/login" && method == "POST") {
		std::size_t json_start = body.find('{');
		std::string json_body = body.substr(json_start);  // Extract JSON bite
		if (json_body != "") {
			try {
				json parsed_body = json::parse(json_body);  // Parse the body as JSON
                std::string email = parsed_body["email"];
				std::string password = parsed_body["password"];
				if (handleDbQuery(socket,
					"SELECT * FROM usersad WHERE email = '" + email + "' AND password = '" + password + "';",
					"Profile data for email: " + email + ", password: " + password + "\n",
					"Login")) 
				{
					handleLoginRequest(socket, email);  //Create Token
					std::cout << "Login OK" << std::endl; // сообщение дл€ теста					
                }
			}
			catch (const std::exception& e) {
				std::cerr << "JSON Parsing error: " << e.what() << std::endl;
			}
		}
        return;
    }

    // Process the headers and look for the token
    std::istringstream body_stream(body);
    std::string header;
    std::string token;
    while (std::getline(body_stream, header)) {
        std::cout << "header: " << header << std::endl;//сообще€ие дл€ теста
        if (header.find("{") != std::string::npos) { break; }
        if (header.find("Cookie:") == 0) {
           // const std::string token_prefix = "Authorization: Bearer ";            
            token = trim(header.substr(22)); //token = header.substr(22, header.length()-23);
            break;
        }
    }
    if (token.empty() || !validateJWT(socket, token)) {
        std::string response = "HTTP/1.1 401 Unauthorized\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "\r\n";
        response += "Invalid or missing token";

        boost::asio::write(socket, boost::asio::buffer(response));
        return;
    }

    // Check if URI starts with "/api/"
    std::string api_prefix = "/api/";
    if (uri.substr(0, api_prefix.length()) != api_prefix) {
        std::string not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
        boost::asio::write(socket, boost::asio::buffer(not_found_response));
        return;
    }    
    std::unordered_map<std::string, int> uri_map = {
       {"/api/cart/", 1},
       {"/api/profile/", 2},
       {"/api/orders/", 3}
    };

    std::string base_uri = uri.substr(0, uri.find('/', api_prefix.length()) + 1);
    int uri_key = uri_map[base_uri];

    long long codd_id = 0;
    std::cout << "base_uri.length(): " << base_uri.length() << " uri.length(): " << uri.length() << std::endl;//сообще€ие дл€ теста
    std::cout << "base_uri: " << base_uri << " uri: " << uri << std::endl;//сообще€ие дл€ теста

    if (base_uri.length() != uri.length()) {
        try {
            codd_id = std::stoll(uri.substr(base_uri.length()));
        }
        catch (...) {
            return; // If out of range
        }
    }
    std::cout << "handleRequest -- uri_key: " << uri_key  << " ============= user_id: " << codd_id << std::endl;//сообще€ие дл€ теста
    
    switch (uri_key) {
        case 1: {  // Processing a Query for a Cart
            handleDbQuery(socket,
                "SELECT * FROM carts WHERE user_id = " + std::to_string(codd_id) + ";",
                "Cart items for user " + std::to_string(codd_id) + ":\n",
                "Json");
            break;
        }
        case 2: {  // ќбработка запроса дл€ профил€ пользовател€
            handleDbQuery(socket,
                "SELECT * FROM users WHERE user_id = " + std::to_string(codd_id) + ";",
                "Profile data for user " + std::to_string(codd_id) + ":\n",
                "Json");
            break;
        }
        case 3: {  // ќбработка запроса дл€ заказов пользовател€
            handleDbQuery(socket,
                "SELECT * FROM orders WHERE user_id = " + std::to_string(codd_id) + ";",
                "Order details for user " + std::to_string(codd_id) + ":\n",
                "Json");
            break;
        }
        default: {  // ≈сли URI не найден, возвращаем 404
            std::string not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
            boost::asio::write(socket, boost::asio::buffer(not_found_response));
            break;
        }
    }
}