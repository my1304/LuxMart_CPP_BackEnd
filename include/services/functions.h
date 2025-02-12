// scr/services/functions.h
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include "bcrypt/bcrypt.h"

std::string trim(const std::string& str);
std::string encode(const std::string& password);
bool matches(const std::string& password, const std::string& hashed);

#endif // FUNCTIONS_H