#include "crow.h"
#include "jwt/jwt.hpp"
#include "AuthService.h"
#include "UserService.h"

// Контроллер авторизации
class AuthController {
private:
    AuthService authService;
    UserService userService;

public:
    AuthController(AuthService authService, UserService userService)
        : authService(authService), userService(userService) {}

    // Обработка логина
    crow::response login(const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            std::string email = body["email"].s();
            std::string password = body["password"].s();

            User user = userService.findByEmail(email);
            if (user.getEmail().empty() || !user.isEnabled() || !authService.checkPassword(password, user.getPassword())) {
                return crow::response(401, "Unauthorized");
            }

            auto tokenDto = authService.login(email, password);
            if (tokenDto.getAccessToken().empty()) {
                return crow::response(400, "Bad Request");
            }

            crow::response res;
            res.add_header("Set-Cookie", "Access-Token=" + tokenDto.getAccessToken() + "; HttpOnly; Path=/");
            res.add_header("Set-Cookie", "Refresh-Token=" + tokenDto.getRefreshToken() + "; HttpOnly; Path=/");
            res.code = 200;
            res.body = crow::json::wvalue{
                {"accessToken", tokenDto.getAccessToken()},
                {"refreshToken", tokenDto.getRefreshToken()}
            }.dump();
            return res;

        } catch (const std::exception& e) {
            return crow::response(500, "Internal Server Error");
        }
    }

    // Получение профиля
    crow::response getProfile(const crow::request& req) {
        try {
            auto userEmail = authService.getCurrentUserEmail(req);
            User user = userService.findByEmail(userEmail);
            if (user.getEmail().empty()) {
                return crow::response(404, "Not Found");
            }

            crow::json::wvalue userProfile;
            userProfile["id"] = user.getId();
            userProfile["email"] = user.getEmail();
            userProfile["firstName"] = user.getFirstName();
            userProfile["lastName"] = user.getLastName();
            return crow::response(200, userProfile);

        } catch (const std::exception& e) {
            return crow::response(500, "Internal Server Error");
        }
    }

    // Получение нового access-токена по refresh-токену
    crow::response refresh(const crow::request& req) {
        try {
            auto cookies = req.get_header_value("Cookie");
            std::string refreshToken = authService.getCookie(cookies, "Refresh-Token");

            TokenResponseDto tokenDto = authService.getAccessToken(refreshToken);
            crow::response res;
            res.add_header("Set-Cookie", "Access-Token=" + tokenDto.getAccessToken() + "; HttpOnly; Path=/");
            res.add_header("Set-Cookie", "Refresh-Token=" + tokenDto.getRefreshToken() + "; HttpOnly; Path=/");
            res.code = 200;
            res.body = crow::json::wvalue{
                {"accessToken", tokenDto.getAccessToken()},
                {"refreshToken", tokenDto.getRefreshToken()}
            }.dump();
            return res;

        } catch (const std::exception& e) {
            return crow::response(400, e.what());
        }
    }

    // Логаут
    crow::response logout(const crow::request& req) {
        crow::response res;
        res.add_header("Set-Cookie", "Access-Token=; HttpOnly; Path=/; Max-Age=0");
        res.add_header("Set-Cookie", "Refresh-Token=; HttpOnly; Path=/; Max-Age=0");
        res.code = 200;
        return res;
    }
};

int main() {
    crow::SimpleApp app;

    AuthService authService;
    UserService userService;
    AuthController authController(authService, userService);

    // Маршруты
    CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)([&](const crow::request& req) {
        return authController.login(req);
    });

    CROW_ROUTE(app, "/api/auth/profile").methods("GET"_method)([&](const crow::request& req) {
        return authController.getProfile(req);
    });

    CROW_ROUTE(app, "/api/auth/refresh").methods("GET"_method)([&](const crow::request& req) {
        return authController.refresh(req);
    });

    CROW_ROUTE(app, "/api/auth/logout").methods("GET"_method)([&](const crow::request& req) {
        return authController.logout(req);
    });

    app.port(8088).multithreaded().run();
    return 0;
}