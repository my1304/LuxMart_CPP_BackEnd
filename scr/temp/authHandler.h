#ifndef AUTH_HANDLER_H
#define AUTH_HANDLER_H

#include <boost/asio.hpp>
#include <string>

void handleProtectedRequest(boost::asio::ip::tcp::socket& socket, const std::string& token);
void authenticateUser(boost::asio::ip::tcp::socket& socket, const std::string& username, const std::string& password);

#endif // AUTH_HANDLER_H