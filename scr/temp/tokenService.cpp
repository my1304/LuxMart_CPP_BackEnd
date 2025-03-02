//  JSON Web Tokens (JWT) � ������������������ ���������
#include <jwt-cpp/jwt.h>
#include <chrono>
#include <string>
#include <iostream>

class TokenService {
private:
    std::string accessKey;
    std::string refreshKey;

public:
    // ������������� � �������
    TokenService(const std::string& accessKey, const std::string& refreshKey)
        : accessKey(accessKey), refreshKey(refreshKey) {}

    // ��������� Access Token
    std::string generateAccessToken(const std::string& username, const std::string& email) {
        auto now = std::chrono::system_clock::now();
        auto expiration = now + std::chrono::hours(24);  // ���� �������� 1 ����

        return jwt::create()
            .set_issuer("luxmart")
            .set_subject(username)
            .set_expires_at(std::chrono::system_clock::to_time_t(expiration))
            .set_payload_claim("email", jwt::claim(email))
            .set_payload_claim("name", jwt::claim(username))
            .sign(jwt::algorithm::hs256{ accessKey });
    }

    // ��������� Refresh Token
    std::string generateRefreshToken(const std::string& username) {
        auto now = std::chrono::system_clock::now();
        auto expiration = now + std::chrono::days(7);  // ���� �������� 7 ����

        return jwt::create()
            .set_issuer("luxmart")
            .set_subject(username)
            .set_expires_at(std::chrono::system_clock::to_time_t(expiration))
            .sign(jwt::algorithm::hs256{ refreshKey });
    }

    // ��������� Access Token
    bool validateAccessToken(const std::string& token) {
        return validateToken(token, accessKey);
    }

    // ��������� Refresh Token
    bool validateRefreshToken(const std::string& token) {
        return validateToken(token, refreshKey);
    }

    // ���������� ������ �� Access Token
    jwt::decoded_jwt<jwt::traits::kazuho_picojson> getAccessClaims(const std::string& token) {
        return getClaims(token, accessKey);
    }

    // ���������� ������ �� Refresh Token
    jwt::decoded_jwt<jwt::traits::kazuho_picojson> getRefreshClaims(const std::string& token) {
        return getClaims(token, refreshKey);
    }

private:
    // ����� ������� ��� ��������� ������
    bool validateToken(const std::string& token, const std::string& key) {
        try {
            auto decoded = jwt::decode(token);
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{ key })
                .with_issuer("luxmart");

            verifier.verify(decoded);
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Token validation failed: " << e.what() << std::endl;
            return false;
        }
    }

    // ����� ������� ��� ���������� claims �� ������
    jwt::decoded_jwt<jwt::traits::kazuho_picojson> getClaims(const std::string& token, const std::string& key) {
        try {
            return jwt::decode(token);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to decode token: " << e.what() << std::endl;
            throw;
        }
    }
};

int main() {
    std::string accessKey = "my_access_key_base64_encoded";
    std::string refreshKey = "my_refresh_key_base64_encoded";

    TokenService tokenService(accessKey, refreshKey);

    std::string username = "user123";
    std::string email = "user@example.com";

    // ��������� Access � Refresh �������
    std::string accessToken = tokenService.generateAccessToken(username, email);
    std::string refreshToken = tokenService.generateRefreshToken(username);

    std::cout << "Access Token: " << accessToken << std::endl;
    std::cout << "Refresh Token: " << refreshToken << std::endl;

    // ��������� �������
    bool isAccessValid = tokenService.validateAccessToken(accessToken);
    bool isRefreshValid = tokenService.validateRefreshToken(refreshToken);

    std::cout << "Access token valid: " << isAccessValid << std::endl;
    std::cout << "Refresh token valid: " << isRefreshValid << std::endl;

    // ��������� claims �� �������
    auto accessClaims = tokenService.getAccessClaims(accessToken);
    std::cout << "Email from Access Token: " << accessClaims.get_payload_claim("email").as_string() << std::endl;

    return 0;
}