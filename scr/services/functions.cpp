#include "services/functions.h"
#include "bcrypt/bcrypt.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
        });

    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
        return std::isspace(ch);
        }).base();

    return (start < end) ? std::string(start, end) : std::string();
}

std::string encode(const std::string& password) {
    return bcrypt::generateHash(password);
}

bool matches(const std::string& password, const std::string& hashed) {
    return bcrypt::validatePassword(password, hashed);
}