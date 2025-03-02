#ifndef JWT_SERVICE_H
#define JWT_SERVICE_H

#include <string>

class JwtService {
public:
    std::string generateAccessToken(const std::string& username);
    std::string generateRefreshToken(const std::string& username);
    bool validateToken(const std::string& token);
    std::string extractUsername(const std::string& token);
};

#endif // JWT_SERVICE_H