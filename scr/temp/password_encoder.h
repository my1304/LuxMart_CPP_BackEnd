#ifndef PASSWORD_ENCODER_H
#define PASSWORD_ENCODER_H

#include "bcrypt.h"
#include <iostream>
#include <string>

// Класс для хеширования и проверки паролей
class PasswordEncoder {
public:
    std::string encode(const std::string& password) {
        return bcrypt::generateHash(password);
    }

    bool matches(const std::string& password, const std::string& hashed) {
        return bcrypt::validatePassword(password, hashed);
    }
};

#endif // PASSWORD_ENCODER_H